// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/MidiFile.h"

#include "common/FileIO.h"
#include "common/logger.h"
#include "common/Math.h"
#include "core/SettingsManager.h"

#include <iostream>
#include <vector>
#include <memory>

struct __attribute__((packed)) midiHeaderChunk {
    char chunkType[4];
    uint32_t length;
    uint16_t format;
    uint16_t ntrks;
    uint16_t division;
};

struct __attribute__((packed)) midiTrackChunk {
    char chunkType[4];
    uint32_t length; //in bytes
};

static uint32_t flip32(uint32_t val)
{
    return ((val & 0x000000FF) << 24) | 
            ((val & 0x0000FF00) << 8) | 
            ((val & 0x00FF0000) >> 8) | 
            ((val & 0xFF000000) >> 24);
}

static uint16_t flip16(uint16_t val)
{
    return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}

namespace slr {

MidiFile::MidiFile(long forcedId) : File(FileType::Midi, forcedId) {

}

MidiFile::~MidiFile() {
    if(_handle.is_open()) _handle.close();
}

bool MidiFile::createTemporary(std::string &path) {
    return false;
}

bool MidiFile::open(std::string &path) {
    //TODO: check that file at path exists
    if(_handle.is_open()) {
        LOG_ERROR("File already opened");
        return false;
    }

    if(path.empty()) {
        LOG_FAIL("Path to file is empty");
        return false;
    }

    if(!Common::FileIO::pathHasExtention(Common::FileIO::Extention::Midi, path)) {
        LOG_FAIL("Extention of %s is not .mid", path.c_str());
        return false;
    }
    
    // Common::FileIO::pathIsValid(path, true);

    _handle.open(path.c_str(), std::ios::binary | std::ios::in);
    if(!_handle.is_open()) {
        LOG_FAIL("Failed to open midi file");
        return false;
    }

    midiHeaderChunk header;
    _handle.read((char*)&header, sizeof(midiHeaderChunk));
    header.length = flip32(header.length);
    header.format = flip16(header.format);
    header.division = flip16(header.division);
    header.ntrks = flip16(header.ntrks);
    
    if(header.chunkType[0] != 'M' &&
        header.chunkType[1] != 'T' &&
        header.chunkType[2] != 'h' && 
        header.chunkType[3] != 'd') 
    {
        LOG_FAIL("Midi File header wrong magic");
        _handle.close();
        return false;
    }

    if(header.ntrks > 1) {
        LOG_FAIL("Multitrack Midi files unsupported yet");
        _handle.close();
        return false;
    }

    _format = static_cast<MidiFileFormat>(header.format);
    _ppqn = header.division;
    bool adjustPpqn = false;
    float ppqnAdj = 1.0f;

    if(_ppqn != SettingsManager::getPpqn()) {
        LOG_WARN("PPQN of %u not matching with system %d in file %s",
            _ppqn, SettingsManager::getPpqn(), path.c_str());
        LOG_WARN("Will be rough adjust");
    
        adjustPpqn = true;
        ppqnAdj = SettingsManager::getPpqn() / _ppqn;
    }

    //TODO: should i store raw chunks?
    for(int i=0; i<header.ntrks; ++i) {
        midiTrackChunk trkchunk;

        _handle.read((char*)&trkchunk, sizeof(midiTrackChunk));
        trkchunk.length = flip32(trkchunk.length);

        if(trkchunk.chunkType[0] != 'M' &&
            trkchunk.chunkType[1] != 'T' &&
            trkchunk.chunkType[2] != 'r' &&
            trkchunk.chunkType[3] != 'k') 
        {
            LOG_FAIL("Midi File Track header wrong magic");
            _handle.close();
            return false;
        }

        // std::unique_ptr<uint8_t[]> readed = std::unique_ptr<uint8_t[]>(new uint8_t[trkchunk.length]); 
        std::unique_ptr<uint8_t[]> readed = std::make_unique<uint8_t[]>(trkchunk.length);
        uint8_t * rawdata = readed.get();
        _handle.read((char*)rawdata, trkchunk.length);
        
        MidiTrack track;
        //track_event = <v_time> + <midi_event> | <meta_event> | <sysex_event>
        int bytesReaded = 0;
        frame_t trackGlobalPosition = 0;
        frame_t currentEventOffset = 0;

        while(bytesReaded < trkchunk.length) {
            frame_t position = readvlv(rawdata, bytesReaded);
            uint8_t byte = rawdata[bytesReaded++];

            MidiEventType evType = static_cast<MidiEventType>(byte);
            if(evType > MidiEventType::InvalidType && evType < MidiEventType::SysEx) {
                //midi event
                MidiEvent ev;
                ev.type = static_cast<MidiEventType>(byte & 0xF0);
                ev.channel = byte & 0x0F;

                int length = expectedLength(evType);
                if(length == 2) {
                    ev.note = rawdata[bytesReaded++];
                    ev.velocity = rawdata[bytesReaded++];
                } else if(length == 1) {
                    ev.note = rawdata[bytesReaded++];
                    ev.velocity = 0;
                } else {
                    LOG_FAIL("Wrong length");
                    break;
                }

                if(!adjustPpqn) ev.offset = position+trackGlobalPosition;
                else ev.offset = sMath::floor((position+trackGlobalPosition) * ppqnAdj);

                track.midiEvents.push_back(ev);
            } else if(evType >= MidiEventType::SysEx && evType <= MidiEventType::SystemReset) {
                //sysex or meta
                if(byte == 0xF0 || byte == 0xF7) {
                    //sysex
                    SysexEvent sev;
                    byte = rawdata[bytesReaded++]; //read first data byte
                    while(byte != 0xF7) {
                        sev.data.push_back(byte);   
                        byte = rawdata[bytesReaded++];
                    }

                    if(!adjustPpqn) sev.offset = position+trackGlobalPosition;
                    else sev.offset = sMath::floor((position+trackGlobalPosition) * ppqnAdj);

                    track.sysexEvents.push_back(std::move(sev));
                } else if(byte == 0xFF) {
                    //meta
                    MetaEvent mev;
                    // byte = rawdata[bytesReaded++];
                    mev.type = static_cast<MetaEventType>(rawdata[bytesReaded++]);
                    frame_t length = readvlv(rawdata, bytesReaded);
                    for(frame_t j=0; j<length; ++j) {
                        mev.data.push_back(rawdata[bytesReaded++]);
                    }

                    if(!adjustPpqn) mev.offset = position+trackGlobalPosition;
                    else mev.offset = sMath::floor((position+trackGlobalPosition) * ppqnAdj);
                    
                    track.metaEvents.push_back(std::move(mev));
                }
            } else {
                //invalid
                LOG_FAIL("Invalid byte in midi file");
                break;
            }

            trackGlobalPosition += position;
        }
        
        if(bytesReaded > trkchunk.length) {
            LOG_FAIL("Readed more than needed! readed %d, length %u", bytesReaded, trkchunk.length);
        }

        // track.length = (trackGlobalPosition / _ppqn) * 22050;
        _tracks.push_back(std::move(track));
    }

    if(_tracks.size() != header.ntrks) { 
        LOG_FAIL("tracks count uneven, failed to parse midi file, readed: %lu, expected: %u", _tracks.size(), header.ntrks);
        _handle.close();
        return false;
    }

    return true;
}

bool MidiFile::save() {
    return false;
}

bool MidiFile::close() {
    return false;
}

void MidiFile::prepareForRecord() {

}

void MidiFile::finishAfterRecord() {

}

frame_t MidiFile::frames() const {
    return _tracks[0].midiEvents.at(_tracks[0].midiEvents.size()-1).offset;
}

bool MidiFile::openInternal(std::string &path, bool tmp) {
    return false;
}
    
frame_t MidiFile::readvlv(uint8_t * data, int &currentReadPosition) {
    frame_t position = 0;
    uint8_t byte = data[currentReadPosition++];
    bool readingvlv = true;
    while(readingvlv) {
        position += byte & 0x7F;
        if(byte & 0x80) {
            //keep reading
            position = position << 7;
            byte = data[currentReadPosition++];
        } else {
            readingvlv = false;
        }
    }
    return position;
}

}