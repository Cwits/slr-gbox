// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <atomic>
#include <cstddef>

namespace slr {
/* some LLM stuff here */
template<typename T, std::size_t Capacity>
struct SPSCQueue {
    bool push(const T& item) {
        const std::size_t next = (_head + 1) & mask;
        if (next == _tailAtomic.load(std::memory_order_acquire))
            return false;

        _buffer[_head] = item;
        _head = next;
        _headAtomic.store(_head, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        const std::size_t tail = _tail;
        if(tail == _headAtomic.load(std::memory_order_acquire))
            return false;

        item = _buffer[tail];
        _tail = (_tail + 1) & mask;
        _tailAtomic.store(_tail, std::memory_order_release);
        return true;
    }

    bool peek(const T*& ptr) const {
        const std::size_t tail = _tail;
        if (tail == _headAtomic.load(std::memory_order_acquire))
            return false;

        ptr = &_buffer[tail];
        return true;
    }

    void commit_pop() {
        _tail = (_tail + 1) & mask;
        _tailAtomic.store(_tail, std::memory_order_release);
    }

    bool empty() const {
        return _tailAtomic.load(std::memory_order_acquire) == _headAtomic.load(std::memory_order_acquire);
    }

private:
    static constexpr std::size_t mask = Capacity - 1;

    alignas(64) std::size_t _head = 0;
    alignas(64) std::size_t _tail = 0;

    alignas(64) std::atomic<std::size_t> _headAtomic{0};
    alignas(64) std::atomic<std::size_t> _tailAtomic{0};

    T _buffer[Capacity];

    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of two");
};

}
