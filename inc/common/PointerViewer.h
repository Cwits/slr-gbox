// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <shared_mutex>
#include <memory>
#include <mutex>
#include "logger.h"


namespace slr {
    class AudioUnitView;
}


namespace Common {

template<typename T>
struct PointerViewer { 
    std::shared_ptr<T> getptr() const {
        // std::shared_lock l(_mutex);
        // return current;
        return std::atomic_load(&current);
    };
    
    void update(std::shared_ptr<T> newptr) {
        std::atomic_store(&current, newptr);
        // std::unique_lock l(_mutex);
        // current = std::move(newptr);
    }
    
    private:
    std::shared_ptr<T> current;
    // mutable std::shared_mutex _mutex;
};

template<typename T, typename U>
std::shared_ptr<T> getCastedTo(const std::shared_ptr<U> &pv) {
    if(pv) {
        std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(pv);
        if(casted) return casted;
        else {
            LOG_ERROR("Failed to cast pointer to type");
        }
    } else {
        LOG_ERROR("Pointer is deprecated");
    }

    return std::shared_ptr<T>();
}

using SafeSharedPtr = std::shared_ptr<PointerViewer<slr::AudioUnitView>>;


}