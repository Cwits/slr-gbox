// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

/* 
    Disclaimer: this shit is total slop, i've even didn't verified that, but!
    this driver will allow me to test and verify (event tho there is klicks, pops and extra noise in sound)
    code without compiling on raspberry pi or dealing with another OS to get the sound out, so yeah... :D
    
    !!! FOR TEST AND DEVELOPMENT PURPOSE ONLY !!!
*/
#include "core/drivers/udpDriver.h"
// #include "../protocol/AudioProtocol.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

/* AudioProtocol.h start*/
// #pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace udp_audio {

constexpr uint32_t MAGIC = 0x55444142; // "UDAB"
constexpr uint16_t VERSION = 2;        // 2: audio blocks get cut into fragments (reserved = idx | count << 8)
constexpr uint16_t DEFAULT_PORT = 39871;

constexpr size_t MAX_DATAGRAM_PAYLOAD = 1400;

enum class Type : uint16_t {
    Hello = 1,
    HelloAck = 2,
    AudioInput = 3,   // Windows -> Linux
    AudioOutput = 4   // Linux -> Windows
};

#pragma pack(push, 1)
struct ControlPacket {
    uint32_t magic = MAGIC;
    uint16_t version = VERSION;
    Type type = Type::Hello;
    uint32_t sampleRate = 48000;
    uint32_t blockSize = 256;
    uint16_t inputs = 2;
    uint16_t outputs = 2;
    uint32_t streamId = 0;
};

struct AudioHeader {
    uint32_t magic = MAGIC;
    uint16_t version = VERSION;
    Type type = Type::AudioInput;
    uint32_t streamId = 0;
    uint32_t sequence = 0;
    uint32_t frames = 0;    // frames in whole block
    uint16_t channels = 0;
    uint16_t reserved = 0;  // LSB: fragment index, MSB: index count
};
#pragma pack(pop)

static_assert(sizeof(ControlPacket) == 24);
static_assert(sizeof(AudioHeader) == 24);

inline uint32_t fragmentCount(uint32_t frames, uint32_t channels) {
    const size_t total = static_cast<size_t>(frames) * channels * sizeof(float);
    const size_t c = (total + MAX_DATAGRAM_PAYLOAD - 1) / MAX_DATAGRAM_PAYLOAD;
    return static_cast<uint32_t>(c ? c : 1);
}

inline uint32_t framesPerFragment(uint32_t frames, uint32_t count) {
    return (frames + count - 1) / count;
}

template <class SendFn>
void sendFragments(AudioHeader h, const float* data,
                   std::vector<uint8_t>& scratch, SendFn&& sendFn) {
    const uint32_t count = fragmentCount(h.frames, h.channels);
    const uint32_t fpf = framesPerFragment(h.frames, count);
    scratch.resize(sizeof(AudioHeader) + static_cast<size_t>(fpf) * h.channels * sizeof(float));

    for (uint32_t idx = 0; idx < count; ++idx) {
        const uint32_t off = idx * fpf;
        if (off >= h.frames) break;
        const uint32_t chunk = std::min(fpf, h.frames - off);
        const size_t bytes = static_cast<size_t>(chunk) * h.channels * sizeof(float);

        h.reserved = static_cast<uint16_t>(idx | (count << 8));
        std::memcpy(scratch.data(), &h, sizeof(h));
        std::memcpy(scratch.data() + sizeof(h),
                    data + static_cast<size_t>(off) * h.channels, bytes);
        sendFn(scratch.data(), sizeof(h) + bytes);
    }
}

class BlockAssembler {
public:
    void configure(uint32_t frames, uint16_t channels) {
        _frames = frames;
        _channels = channels;
        _block.assign(static_cast<size_t>(frames) * channels, 0.0f);
        _active = false;
        _haveLast = false;
        _lost = 0;
    }

    // true - блок полностью собран (data() / sequence()).
    bool push(const AudioHeader& h, const uint8_t* payload, size_t payloadBytes) {
        const uint32_t idx = h.reserved & 0xFF;
        const uint32_t count = h.reserved >> 8;
        if (_block.empty() || h.frames != _frames || h.channels != _channels ||
            count == 0 || count > 64 || idx >= count ||
            count != fragmentCount(_frames, _channels))
            return false;

        const uint32_t fpf = framesPerFragment(_frames, count);
        const uint32_t off = idx * fpf;
        if (off >= _frames) return false;
        const uint32_t chunk = std::min(fpf, _frames - off);
        if (payloadBytes != static_cast<size_t>(chunk) * _channels * sizeof(float))
            return false;

        if (_haveLast && static_cast<int32_t>(h.sequence - _lastSeq) <= 0)
            return false;                                   // duplicate/outdated 
        if (_active && h.sequence != _curSeq) {
            if (static_cast<int32_t>(h.sequence - _curSeq) < 0)
                return false;                               // late arrived piece of old block
            _active = false;                                // old block fragmented
        }
        if (!_active) {
            _active = true;
            _curSeq = h.sequence;
            _mask = 0;
            _got = 0;
        }

        const uint64_t bit = 1ull << idx;
        if (_mask & bit) return false;
        _mask |= bit;
        ++_got;
        std::memcpy(_block.data() + static_cast<size_t>(off) * _channels, payload, payloadBytes);
        if (_got < count) return false;

        if (_haveLast) {
            const int32_t diff = static_cast<int32_t>(h.sequence - _lastSeq);
            if (diff > 1) _lost += static_cast<uint32_t>(diff - 1);
        }
        _haveLast = true;
        _lastSeq = h.sequence;
        _active = false;
        return true;
    }

    const float* data() const { return _block.data(); }
    uint32_t sequence() const { return _lastSeq; }
    uint32_t takeLost() { const uint32_t v = _lost; _lost = 0; return v; }

private:
    std::vector<float> _block;
    uint32_t _frames = 0;
    uint16_t _channels = 0;
    bool _active = false, _haveLast = false;
    uint32_t _curSeq = 0, _lastSeq = 0, _lost = 0, _got = 0;
    uint64_t _mask = 0;
};

} // namespace udp_audio

/* AudioProtocol.h end*/


namespace {
using Clock = std::chrono::steady_clock;


constexpr size_t kRxRing    = 16; // 
constexpr size_t kPrefill   = 8;  // how much before passing to slr 
constexpr size_t kHighWater = 10; // how much before drop data
constexpr size_t kTrimTo    = 5;  // trim to on overrun
// ---- Keepalive ----
constexpr int kPingIntervalMs = 500;

constexpr size_t kMaxPayload = 59000; //in bytes

bool sendAll(int fd, const sockaddr_in& peer, const void* data, size_t size) {
    return ::sendto(fd, data, size, 0,
        reinterpret_cast<const sockaddr*>(&peer), sizeof(peer)) ==
        static_cast<ssize_t>(size);
}

bool getDefaultGateway(std::string& address) {
    FILE* pipe = popen("ip route show default", "r");
    if (!pipe)
        return false;

    char buffer[256];
    bool found = false;

    if (fgets(buffer, sizeof(buffer), pipe)) {
        char gateway[64]{};
        if (sscanf(buffer, "default via %63s", gateway) == 1) {
            address = gateway;
            found = true;
        }
    }

    pclose(pipe);
    return found;
}


//look for ip
std::vector<std::string> hostCandidates() {
    std::vector<std::string> list;
    auto add = [&](const std::string& h) {
        if (!h.empty() && std::find(list.begin(), list.end(), h) == list.end())
            list.push_back(h);
    };

    if (const char* env = std::getenv("UDP_AUDIO_HOST"))
        add(env);

    std::string gw;
    if (getDefaultGateway(gw))
        add(gw);

    add("127.0.0.1");
    return list;
}

bool resolveHost(const std::string& host, uint16_t port, sockaddr_in& out) {
    out = {};
    out.sin_family = AF_INET;
    out.sin_port = htons(port);

    if (::inet_pton(AF_INET, host.c_str(), &out.sin_addr) == 1)
        return true;

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* result = nullptr;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0 || !result)
        return false;
    out.sin_addr = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr;
    freeaddrinfo(result);
    return true;
}
} // namespace

namespace slr {

UDPDriver::UDPDriver() { }
UDPDriver::~UDPDriver() {
    shutdown();
}

bool UDPDriver::handshake() {
    udp_audio::ControlPacket hello{};
    hello.type = udp_audio::Type::Hello;
    hello.sampleRate = static_cast<uint32_t>(_sampleRate);
    hello.blockSize = static_cast<uint32_t>(_bufferSize);
    hello.inputs = static_cast<uint16_t>(_numInputs);
    hello.outputs = static_cast<uint16_t>(_numOutputs);

    const auto hosts = hostCandidates();

    for (const auto& host : hosts) {
        sockaddr_in server{};
        if (!resolveHost(host, udp_audio::DEFAULT_PORT, server))
            continue;

        std::cerr << "UDPDriver: trying " << host << ":"
                  << udp_audio::DEFAULT_PORT << "\n";

        for (int attempt = 0; attempt < 5; ++attempt) {
            sendAll(_socket, server, &hello, sizeof(hello));

            const auto deadline = Clock::now() + std::chrono::milliseconds(300);
            for (;;) {
                const auto now = Clock::now();
                if (now >= deadline) break;
                const auto remainUs = std::chrono::duration_cast<std::chrono::microseconds>(
                    deadline - now).count();

                fd_set readSet;
                FD_ZERO(&readSet);
                FD_SET(_socket, &readSet);
                timeval wait{};
                wait.tv_sec = static_cast<time_t>(remainUs / 1000000);
                wait.tv_usec = static_cast<suseconds_t>(remainUs % 1000000);
                if (select(_socket + 1, &readSet, nullptr, nullptr, &wait) <= 0)
                    break;

                udp_audio::ControlPacket ack{};
                sockaddr_in from{};
                socklen_t fromLen = sizeof(from);
                const auto n = recvfrom(_socket, &ack, sizeof(ack), 0,
                    reinterpret_cast<sockaddr*>(&from), &fromLen);
                if (n != static_cast<ssize_t>(sizeof(ack)))
                    continue;
                if (from.sin_addr.s_addr != server.sin_addr.s_addr ||
                    from.sin_port != server.sin_port)
                    continue;
                if (ack.magic != udp_audio::MAGIC ||
                    ack.version != udp_audio::VERSION ||
                    ack.type != udp_audio::Type::HelloAck)
                    continue;

                if (ack.sampleRate != hello.sampleRate ||
                    ack.blockSize != hello.blockSize ||
                    ack.inputs != hello.inputs || ack.outputs != hello.outputs) {
                    std::cerr << "UDPDriver: server rejected the request (server: "
                              << ack.sampleRate << " Hz / " << ack.blockSize
                              << " frames; 0 = WASAPI failed to open). Check server log.\n";
                    return false;
                }

                // Фиксируем адрес сервера для аудиопакетов.
                if (::connect(_socket, reinterpret_cast<sockaddr*>(&server), sizeof(server)) != 0)
                    return false;
                _streamId = ack.streamId;
                LOG_SUCCESS("Host ip: %s", host.c_str());
                return true;
            }
        }
    }

    std::cerr << "UDPDriver: no response from Windows audio server "
                 "(check firewall UDP " << udp_audio::DEFAULT_PORT
              << ", or set UDP_AUDIO_HOST)\n";
    return false;
}

bool UDPDriver::init(frame_t sampleRate, frame_t bufferSize,
                     int numInputs, int numOutputs) {
    if (_socket >= 0 || _isRunning || sampleRate == 0 || bufferSize == 0 ||
        numInputs < 0 || numOutputs < 0 || numInputs > 16 || numOutputs > 16)
        return false;

    const size_t payload = static_cast<size_t>(bufferSize) *
        static_cast<size_t>(std::max(numInputs, numOutputs)) * sizeof(float);
    if (payload > kMaxPayload)
        return false;

    _sampleRate = sampleRate;
    _bufferSize = bufferSize;
    _numInputs = numInputs;
    _numOutputs = numOutputs;
    _framesPassed = 0;
    _txSequence = 0;
    _rxSequence = 0;
    _streamId = 0;

    _socket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket < 0) return false;

    // Большие буферы сокета, чтобы пережить кратковременные задержки потока.
    const int bufBytes = 1 << 20;
    setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, &bufBytes, sizeof(bufBytes));
    setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, &bufBytes, sizeof(bufBytes));

    // Неблокирующий сокет; handshake ждёт ответ через select().
    const int flags = fcntl(_socket, F_GETFL, 0);
    fcntl(_socket, F_SETFL, flags | O_NONBLOCK);

    if (!handshake()) {
        closeSocket();
        return false;
    }

    const size_t blockSamples = static_cast<size_t>(_bufferSize) * _numInputs;
    _txInterleaved.assign(static_cast<size_t>(_bufferSize) * _numOutputs, 0.0f);
    _rxInterleaved.assign(blockSamples * kRxRing, 0.0f);
    _rxHead = 0;
    _rxCount = 0;
    return true;
}

bool UDPDriver::start(AudioCallback clb) {
    if (_socket < 0 || _isRunning || !clb) return false;
    _callback = std::move(clb);
    _threadRunning = true;
    _isRunning = true;
    _thread = std::thread(&UDPDriver::run, this);
    return true;
}

void UDPDriver::run() {
    AudioBuffer input(static_cast<size_t>(_bufferSize), static_cast<size_t>(_numInputs));
    AudioBuffer output(static_cast<size_t>(_bufferSize), static_cast<size_t>(_numOutputs));

    const size_t blockSamples = static_cast<size_t>(_bufferSize) * _numInputs;

    std::vector<uint8_t> packet(4096); 
    std::vector<uint8_t> txScratch;
    txScratch.reserve(2048);

    udp_audio::BlockAssembler assembler;
    if (_numInputs > 0)
        assembler.configure(static_cast<uint32_t>(_bufferSize),
                            static_cast<uint16_t>(_numInputs));

    const auto period = std::chrono::duration_cast<Clock::duration>(
        std::chrono::duration<double>(
            static_cast<double>(_bufferSize) / static_cast<double>(_sampleRate)));

    const bool showStats = std::getenv("UDP_AUDIO_STATS") != nullptr;
    uint32_t stDropped = 0, stUnderrun = 0, stLost = 0;
    auto lastStats = Clock::now();

    auto next = Clock::now();
    auto lastPing = next;

    _rxHead = 0;
    _rxCount = 0;
    bool primed = false;

    while (_threadRunning.load()) {
        // ---- 1. parse socket  ----
        for (;;) {
            const auto n = recv(_socket, packet.data(), packet.size(), 0);
            if (n < 0) break; // EAGAIN / ECONNREFUSED - больше данных нет
            if (n < static_cast<ssize_t>(sizeof(udp_audio::AudioHeader))) continue;

            udp_audio::AudioHeader header{};
            std::memcpy(&header, packet.data(), sizeof(header));

            if (header.magic != udp_audio::MAGIC ||
                header.version != udp_audio::VERSION ||
                header.type != udp_audio::Type::AudioInput) // HelloAck
                continue;
            if (_numInputs == 0 || header.streamId != _streamId) continue;

            if (!assembler.push(header, packet.data() + sizeof(header),
                                static_cast<size_t>(n) - sizeof(header)))
                continue; // block not finished

            stLost += assembler.takeLost();

            if (_rxCount == kRxRing) { // drop old data
                _rxHead = (_rxHead + 1) % kRxRing;
                --_rxCount;
                ++stDropped;
            }
            const size_t slot = (_rxHead + _rxCount) % kRxRing;
            std::memcpy(_rxInterleaved.data() + slot * blockSamples,
                        assembler.data(), blockSamples * sizeof(float));
            ++_rxCount;
        }

        // jitter, cut to min.
        if (_rxCount > kHighWater) {
            const size_t drop = _rxCount - kTrimTo;
            _rxHead = (_rxHead + drop) % kRxRing;
            _rxCount -= drop;
            stDropped += static_cast<uint32_t>(drop);
        }

        // ---- 2. give the block out ----
        if (_numInputs > 0) {
            if (!primed && _rxCount >= kPrefill) primed = true;
            if (primed && _rxCount == 0) { primed = false; ++stUnderrun; }

            if (primed) {
                const float* blk = _rxInterleaved.data() + _rxHead * blockSamples;
                for (int c = 0; c < _numInputs; ++c)
                    for (size_t f = 0; f < _bufferSize; ++f)
                        input[static_cast<size_t>(c)][f] =
                            blk[f * static_cast<size_t>(_numInputs) + c];
                _rxHead = (_rxHead + 1) % kRxRing;
                --_rxCount;
            } else {
                for (int c = 0; c < _numInputs; ++c)
                    std::fill(input[static_cast<size_t>(c)],
                              input[static_cast<size_t>(c)] + _bufferSize, 0.0f);
            }
        }

        for (int c = 0; c < _numOutputs; ++c)
            std::fill(output[static_cast<size_t>(c)],
                      output[static_cast<size_t>(c)] + _bufferSize, 0.0f);

        if (_callback)
            _callback(_numInputs ? &input : nullptr,
                      _numOutputs ? &output : nullptr,
                      _bufferSize, _framesPassed);

        // ---- 3. send data to server  ----
        if (_numOutputs > 0) {
            for (size_t f = 0; f < _bufferSize; ++f)
                for (int c = 0; c < _numOutputs; ++c)
                    _txInterleaved[f * static_cast<size_t>(_numOutputs) + c] =
                        output[static_cast<size_t>(c)][f];

            udp_audio::AudioHeader header{};
            header.type = udp_audio::Type::AudioOutput;
            header.streamId = _streamId;
            header.sequence = _txSequence++;
            header.frames = static_cast<uint32_t>(_bufferSize);
            header.channels = static_cast<uint16_t>(_numOutputs);

            udp_audio::sendFragments(header, _txInterleaved.data(), txScratch,
                [&](const uint8_t* d, size_t len) { ::send(_socket, d, len, 0); });
        }

        // ---- 4. Keepalive:  ----
        {
            const auto now = Clock::now();
            if (now - lastPing > std::chrono::milliseconds(kPingIntervalMs)) {
                udp_audio::ControlPacket ping{};
                ping.type = udp_audio::Type::Hello;
                ping.sampleRate = static_cast<uint32_t>(_sampleRate);
                ping.blockSize = static_cast<uint32_t>(_bufferSize);
                ping.inputs = static_cast<uint16_t>(_numInputs);
                ping.outputs = static_cast<uint16_t>(_numOutputs);
                ping.streamId = _streamId;
                ::send(_socket, &ping, sizeof(ping), 0);
                lastPing = now;
            }

            if (showStats && now - lastStats > std::chrono::seconds(1)) {
                std::fprintf(stderr,
                    "UDPDriver stats: queue=%zu dropped=%u underrun=%u lost=%u\n",
                    _rxCount, stDropped, stUnderrun, stLost);
                lastStats = now;
            }
        }

        // ---- 5. advance clock ----
        _framesPassed += _bufferSize;
        next += period;
        const auto now = Clock::now();
        if (now - next > period * 4)
            next = now;                       // resync
        else
            std::this_thread::sleep_until(next);
    }
    _isRunning = false;
}

bool UDPDriver::stop() {
    if (!_isRunning) return true;
    _threadRunning = false;
    if (_thread.joinable()) _thread.join();
    _isRunning = false;
    return true;
}

void UDPDriver::closeSocket() {
    if (_socket >= 0) {
        ::close(_socket);
        _socket = -1;
    }
}

bool UDPDriver::shutdown() {
    stop();
    closeSocket();
    _callback = {};
    _txInterleaved.clear();
    _rxInterleaved.clear();
    _rxHead = 0;
    _rxCount = 0;
    return true;
}

bool UDPDriver::restart() {
    const bool wasRunning = _isRunning;
    const auto sr = _sampleRate;
    const auto bs = _bufferSize;
    const auto in = _numInputs;
    const auto out = _numOutputs;
    auto callback = _callback;
    shutdown();
    if (!init(sr, bs, in, out)) return false;
    return !wasRunning || start(std::move(callback));
}

bool UDPDriver::changeParameters(frame_t sampleRate, frame_t bufferSize,
                                 int numInputs, int numOutputs) {
    const bool wasRunning = _isRunning;
    auto callback = _callback;
    shutdown();
    if (!init(sampleRate, bufferSize, numInputs, numOutputs)) return false;
    return !wasRunning || start(std::move(callback));
}

frame_t UDPDriver::inputLatency(int port) const {
    return (port >= 0 && port < _numInputs) ? _bufferSize * kPrefill : 0;
}

frame_t UDPDriver::outputLatency(int port) const {
    return (port >= 0 && port < _numOutputs) ? _bufferSize : 0;
}

}