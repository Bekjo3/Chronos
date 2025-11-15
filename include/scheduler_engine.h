#ifndef CHRONOS_SCHEDULER_ENGINE_H
#define CHRONOS_SCHEDULER_ENGINE_H

#include "job.h"
#include "scheduling_policy.h"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

namespace chronos {

// Forward declaration
class WorkerPool;

struct ScheduleResult {
    std::vector<Job> completed_jobs;
    float total_waiting_time = 0.0f;
    float total_turnaround_time = 0.0f;
    float cpu_active_time = 0.0f;
    float idle_time = 0.0f;
    float makespan = 0.0f;
    int num_cores = 1;
    std::size_t dispatch_count = 0;
    std::size_t context_switches = 0;

    float averageWaitingTime() const {
        return completed_jobs.empty()
                   ? 0.0f
                   : total_waiting_time / static_cast<float>(completed_jobs.size());
    }

    float averageTurnaroundTime() const {
        return completed_jobs.empty()
                   ? 0.0f
                   : total_turnaround_time / static_cast<float>(completed_jobs.size());
    }

    float cpuUtilization() const {
        if (makespan <= 0.0f || num_cores <= 0) {
            return 0.0f;
        }
        // For multi-core: utilization = total_cpu_time / (makespan * num_cores)
        return std::min(1.0f, cpu_active_time / (makespan * static_cast<float>(num_cores)));
    }

    std::size_t contextSwitches() const {
        return context_switches;
    }
};

class SchedulerEngine {
public:
    // Run jobs using multithreaded worker pool
    ScheduleResult run(std::vector<Job> jobs, ISchedulingPolicy& policy, int num_cores);

    // Print a summary table and aggregate metrics.
    void printSummary(const ScheduleResult& result, const ISchedulingPolicy& policy) const;

private:
    void printJobTable(const std::vector<Job>& jobs) const;
    
    // Scheduler thread function - dispatches jobs to ready queue
    void schedulerThread(std::vector<Job> jobs, ISchedulingPolicy& policy, 
                       std::vector<Job>& ready_queue,
                       std::mutex& queue_mutex,
                       std::condition_variable& job_available,
                       std::atomic<bool>& simulation_running,
                       ScheduleResult& result,
                       WorkerPool& worker_pool);
    
    // Collect completed jobs from ready queue
    void collectCompletedJobs(std::vector<Job>& ready_queue, 
                            std::vector<Job>& completed_jobs,
                            std::mutex& queue_mutex);
};

}

#endif