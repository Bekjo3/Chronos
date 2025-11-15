#ifndef CHRONOS_METRICS_COLLECTOR_H
#define CHRONOS_METRICS_COLLECTOR_H

#include "job.h"

#include <cstddef>
#include <mutex>
#include <vector>

namespace chronos {

// Collects and aggregates scheduling performance metrics
class MetricsCollector {
public:
    MetricsCollector();
    
    void recordJobCompletion(const Job& job);
    
    void recordCpuActivity(float duration);
    
    void recordIdleTime(float duration);
    
    void recordContextSwitch();
    
    // Set the total simulation makespan
    void setMakespan(float makespan);
    
    void setCpuActiveTime(float cpu_time);
    
    void setContextSwitches(std::size_t switches);
    
    void setNumCores(int cores);
    
    const std::vector<Job>& getCompletedJobs() const { return completed_jobs_; }
    
    float getTotalWaitingTime() const { return total_waiting_time_; }
    
    float getTotalTurnaroundTime() const { return total_turnaround_time_; }
    
    float getAverageWaitingTime() const;
    
    float getAverageTurnaroundTime() const;
    
    // Get CPU utilization (percentage as decimal, 0.0 to 1.0)
    float getCpuUtilization() const;
    
    // Get CPU utilization as percentage (0.0 to 100.0)
    float getCpuUtilizationPercent() const { return getCpuUtilization() * 100.0f; }
    
    std::size_t getContextSwitches() const;
    
    // Get total number of completed jobs
    std::size_t getJobCount() const { return completed_jobs_.size(); }
    
    // Get makespan (total simulation time)
    float getMakespan() const { return makespan_; }
    
    float getTotalCpuActiveTime() const { return cpu_active_time_; }
    
    float getTotalIdleTime() const { return idle_time_; }
    
    // Reset all metrics (for reuse)
    void reset();
    
    // Thread-safe version of recordJobCompletion (for multithreaded use)
    void recordJobCompletionThreadSafe(const Job& job);
    
    // Thread-safe version of recordContextSwitch
    void recordContextSwitchThreadSafe();

private:
    std::vector<Job> completed_jobs_;
    float total_waiting_time_;
    float total_turnaround_time_;
    float cpu_active_time_;
    float idle_time_;
    float makespan_;
    int num_cores_;
    std::size_t dispatch_count_;
    
    mutable std::mutex metrics_mutex_;
    
    // Helper to calculate averages safely
    float calculateAverage(float total, std::size_t count) const;
};

} 

#endif