// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <thread>
#include <array>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <memory>
#include <vector>

namespace Profiler {

using tp = std::chrono::time_point<std::chrono::steady_clock>;
using reading = std::pair<tp, tp>;
constexpr int READINGS_COUNT = 32;

struct ReadingResults {
    std::string_view _threadName;
    std::thread::id _threadId;

    int _avg;
    int _max;
    int _min;
    int _last;

    std::array<reading, READINGS_COUNT> _raw;
};

struct ResultQueue {
    ResultQueue(const std::string name);
    ResultQueue(const std::string name, const std::thread::id id);

    void start(tp point);
    void end(tp point);
    
    std::array<reading, READINGS_COUNT> results() const { return _readings; }
    
    int last() const;

    const std::string_view name() const { return _name; }
    const std::thread::id id() const { return _id; }

    private:
    std::string _name;
    std::thread::id _id;

    std::atomic<std::size_t> _head;
    std::array<reading, READINGS_COUNT> _readings;
    reading _current;

    void init();
};

ResultQueue * prepare(std::string name);
void start(ResultQueue *q);
void end(ResultQueue *q);
const std::vector<ReadingResults> results();

}