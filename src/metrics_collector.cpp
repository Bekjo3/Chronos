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
    , num_cores_(1)
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

void MetricsCollector::setCpuActiveTime(float cpu_time) {
    cpu_active_time_ = cpu_time;
}

void MetricsCollector::setContextSwitches(std::size_t switches) {
    dispatch_count_ = switches;
}

void MetricsCollector::setNumCores(int cores) {
    num_cores_ = cores > 0 ? cores : 1;
}

float MetricsCollector::getAverageWaitingTime() const {
    return calculateAverage(total_waiting_time_, completed_jobs_.size());
}

float MetricsCollector::getAverageTurnaroundTime() const {
    return calculateAverage(total_turnaround_time_, completed_jobs_.size());
}

float MetricsCollector::getCpuUtilization() const {
    if (makespan_ <= 0.0f || num_cores_ <= 0) {
        return 0.0f;
    }
    // CPU utilization = active time / (total time * num_cores)
    // For multi-core systems, we consider the total CPU time available
    // Example: 2 cores busy for 10s = 20s CPU time, makespan 10s
    //          Utilization = 20 / (10 * 2) = 1.0 = 100%
    return std::min(1.0f, cpu_active_time_ / (makespan_ * static_cast<float>(num_cores_)));
}

std::size_t MetricsCollector::getContextSwitches() const {
    // Context switches = dispatches - num_cores (initial dispatches to each core don't count)
    // Example: 2 cores, 5 jobs → 5 dispatches → 5 - 2 = 3 context switches
    if (dispatch_count_ <= static_cast<std::size_t>(num_cores_)) {
        return 0;
    }
    return dispatch_count_ - static_cast<std::size_t>(num_cores_);
}

void MetricsCollector::reset() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    completed_jobs_.clear();
    total_waiting_time_ = 0.0f;
    total_turnaround_time_ = 0.0f;
    cpu_active_time_ = 0.0f;
    idle_time_ = 0.0f;
    makespan_ = 0.0f;
    num_cores_ = 1;
    dispatch_count_ = 0;
}

float MetricsCollector::calculateAverage(float total, std::size_t count) const {
    if (count == 0) {
        return 0.0f;
    }
    return total / static_cast<float>(count);
}

}