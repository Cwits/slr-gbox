// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/Profiler.h"

namespace Profiler {

std::vector<std::unique_ptr<ResultQueue>> _queues;
std::mutex _profilerLock;

ResultQueue::ResultQueue(const std::string name) :
        _name(name) 
    {
        _id = std::this_thread::get_id();
        init();
    }

ResultQueue::ResultQueue(const std::string name, const std::thread::id id) :
        _name(name),
        _id(id) 
    {
        init();
    }

    void ResultQueue::start(tp point) {
        _current.first = std::move(point);
    }

    void ResultQueue::end(tp point) {
        _current.second = std::move(point);

        std::size_t head = _head.load(std::memory_order_acquire);
        _readings[head] = std::move(_current);

        head += 1;
        if(head >= READINGS_COUNT) head = 0;
        _head.store(head, std::memory_order_release);
    }
    
    int ResultQueue::last() const {
        std::size_t head = _head.load(std::memory_order_acquire);
        if(head == 0) head = READINGS_COUNT-1;
        else head -= 1;
        reading last = _readings[head];

        return std::chrono::duration_cast<std::chrono::microseconds>(
            last.second - last.first
        ).count();
    }

    void ResultQueue::init() {
        _head = 0;
        std::fill(
            _readings.begin(), 
            _readings.end(), 
            reading{tp{}, tp{}});
    }

ResultQueue * prepare(std::string name) {
    std::unique_ptr<ResultQueue> q = std::make_unique<ResultQueue>(name);

    ResultQueue *ret = q.get();
    std::lock_guard<std::mutex> l(_profilerLock);
    _queues.push_back(std::move(q));
    return ret;
}

void start(ResultQueue *q) {
    q->start(std::chrono::steady_clock::now());
}

void end(ResultQueue *q) {
    q->end(std::chrono::steady_clock::now());
}

const std::vector<ReadingResults> results() {
    std::vector<const ResultQueue*> queues;
    {
        std::lock_guard<std::mutex> l(_profilerLock);
        queues.reserve(_queues.size());
        for(const auto &q : _queues) {
            queues.push_back(q.get());
        }
    }

    std::vector<ReadingResults> ret;
    ret.reserve(queues.size());

    for(const auto &q : queues) {
        ReadingResults res;
        res._threadName = q->name();
        res._threadId = q->id();
        res._raw = q->results();

        
		int avg = 0;
		int max = 0;
		int min = 0;
		for(int i=0; i<READINGS_COUNT; ++i) {
			reading &r = res._raw[i];
			int diff = std::chrono::duration_cast<std::chrono::microseconds>(r.second - r.first).count();
			avg += diff;
			max = std::max(max, diff);
			min = std::min(min, diff);
		}
		avg /= READINGS_COUNT;
		
		res._max = max;
		res._min = min;
		res._avg = avg;
		res._last = q->last();
		
		ret.push_back(res);
    }

    return ret;
}


}