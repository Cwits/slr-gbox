// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <string>
#include <atomic>
#include "defines.h"
namespace slr {

enum class FileType { Error = -1, Audio, Midi, AudioPeak };

class File {
    public:
    File();
    File(FileType type);
    virtual ~File();

    //temporary meaned for recording only -> creates new file
    // virtual bool createTemporary(std::string & path) = 0;
    virtual bool open(std::string & path) = 0;
    virtual bool save() = 0;
    virtual bool close() = 0;

    virtual void prepareForRecord() = 0;
    virtual void finishAfterRecord() = 0;

    const FileType type() const { return _type; }
    const bool isAudio() const { return _type == FileType::Audio ? true : false; }
    const bool isMidi() const { return _type == FileType::Midi ? true : false; }
    const bool isAudioPeak() const { return _type == FileType::AudioPeak ? true : false; }
    const ID id() const { return _uniqueId; }

    void setName(std::string & name) { _name = name; } 
    const std::string & name() const { return _name; }

    //path must be full(contains name as well)
    void setPath(std::string & path) { _path = path; }
    //path is full(contains name as well)
    const std::string & path() const { return _path; }

    const bool dirty() const { return _dirty; }
    void markDirty() { _dirty = true; }

    //e.g. when you need to manipulate audio buffer of file
    //you set it offline, manipulate by file worker and than set back online, or not???
    const bool offline() const { return _offline; }
    void setOffline() { _offline = true; }
    void setOnline() { _offline = false; }

    virtual const frame_t frames() const = 0;

    protected:
    std::string _name;
    std::string _path;

    void clearDirty() { _dirty = false; }

    private:
    FileType _type;
    ID _uniqueId;
    std::atomic<bool> _dirty;

    bool _offline;
};
    
}