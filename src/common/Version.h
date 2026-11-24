// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <atomic>

namespace Common {

struct Version {
    uint64_t version() const { return _version; }
    bool compareAndSave(uint64_t other) {
        if(_version != other) {
            _version = other;
            return true;
        } else {
            return false;
        }
    }
    
    protected:
    void incrementVersion() { _version++; }

    private:
    uint64_t _version = 0;
};

struct VersionAtomic {
    uint64_t version() const { return _version.load(std::memory_order_acquire); }

    protected:
    void incrementVersion() { _version.fetch_add(1, std::memory_order_release); }
    
    private:
    std::atomic<uint64_t> _version = 1;
};

}