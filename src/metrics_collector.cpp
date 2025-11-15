#include "metrics_collector.h"

#include <algorithm>
#include <cmath>

namespace chronos {

MetricsCollector::MetricsCollector()
    : total_waiting_time_(0.0f)
    , total_turnaround_time_(0.0f)
    , cpu_active_time_(0.0f)
    , idle_time_(0.0f)
    , makespan_(0.0f)
    , dispatch_count_(0)
{
}

void MetricsCollector::recordJobCompletion(const Job& job) {
    completed_jobs_.push_back(job);
    total_waiting_time_ += job.getWaitingTime();
    total_turnaround_time_ += job.getTurnaroundTime();
}

void MetricsCollector::recordJobCompletionThreadSafe(const Job& job) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    recordJobCompletion(job);
}

void MetricsCollector::recordCpuActivity(float duration) {
    if (duration > 0.0f) {
        cpu_active_time_ += duration;
    }
}

void MetricsCollector::recordIdleTime(float duration) {
    if (duration > 0.0f) {
        idle_time_ += duration;
    }
}

void MetricsCollector::recordContextSwitch() {
    dispatch_count_++;
}

void MetricsCollector::recordContextSwitchThreadSafe() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    recordContextSwitch();
}

void MetricsCollector::setMakespan(float makespan) {
    makespan_ = makespan;
}

float MetricsCollector::getAverageWaitingTime() const {
    return calculateAverage(total_waiting_time_, completed_jobs_.size());
}

float MetricsCollector::getAverageTurnaroundTime() const {
    return calculateAverage(total_turnaround_time_, completed_jobs_.size());
}

float MetricsCollector::getCpuUtilization() const {
    if (makespan_ <= 0.0f) {
        return 0.0f;
    }
    // CPU utilization = active time / total time
    // For multi-core systems, we consider the total CPU time available
    // Utilization = cpu_active_time / (makespan * num_cores) would be more accurate
    // but for now, we use simple utilization = active_time / makespan
    return std::min(1.0f, cpu_active_time_ / makespan_);

    /*
    Cores	Active                      time	    Makespan	Formula	Result
    1	     8 s	                    10 s	     8 / 10	    0.8 (80 %)
    4	     32 s (sum of 4 cores)	    10 s	  32 / (10×4)	0.8 (80 %)

    Without that correction, the 4-core system would report 32 / 10 = 3.2 → 320 %
    */
}

std::size_t MetricsCollector::getContextSwitches() const {
    // Context switches = number of dispatches - 1 (first dispatch doesn't count as a switch)
    return dispatch_count_ > 0 ? dispatch_count_ - 1 : 0;
}

void MetricsCollector::reset() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    completed_jobs_.clear();
    total_waiting_time_ = 0.0f;
    total_turnaround_time_ = 0.0f;
    cpu_active_time_ = 0.0f;
    idle_time_ = 0.0f;
    makespan_ = 0.0f;
    dispatch_count_ = 0;
}

float MetricsCollector::calculateAverage(float total, std::size_t count) const {
    if (count == 0) {
        return 0.0f;
    }
    return total / static_cast<float>(count);
}

}