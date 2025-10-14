// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <atomic>

namespace slr {

template<typename T, size_t Capacity>
class SPSCQueue {
    public:
    bool push(const T& item) {
        std::size_t nextHead = (head + 1) % Capacity;
        if (nextHead == tail.load(std::memory_order_acquire))
            return false; // full
        buffer[head] = item;
        head.store(nextHead, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        if (tail == head.load(std::memory_order_acquire))
            return false; // empty
        item = buffer[tail];
        tail.store((tail + 1) % Capacity, std::memory_order_release);

        return true;
    }

    private:
    std::atomic<size_t> head = 0;
    std::atomic<size_t> tail = 0;
    T buffer[Capacity];

};

}