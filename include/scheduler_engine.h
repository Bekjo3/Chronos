#ifndef CHRONOS_SCHEDULER_ENGINE_H
#define CHRONOS_SCHEDULER_ENGINE_H

#include "job.h"
#include "scheduling_policy.h"

#include <cstddef>
#include <vector>

namespace chronos {

struct ScheduleResult {
    std::vector<Job> completed_jobs;
    float total_waiting_time = 0.0f;
    float total_turnaround_time = 0.0f;
    float cpu_active_time = 0.0f;
    float idle_time = 0.0f;
    float makespan = 0.0f;
    std::size_t dispatch_count = 0;

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
        return makespan <= 0.0f ? 0.0f : cpu_active_time / makespan;
    }

    std::size_t contextSwitches() const {
        return dispatch_count > 0 ? dispatch_count - 1 : 0;
    }
};

class SchedulerEngine {
public:
    // Run jobs sequentially using the supplied scheduling policy.
    ScheduleResult run(std::vector<Job> jobs, ISchedulingPolicy& policy);

    // Print a summary table and aggregate metrics.
    void printSummary(const ScheduleResult& result, const ISchedulingPolicy& policy) const;

private:
    void printJobTable(const std::vector<Job>& jobs) const;
};

} 

#endif 