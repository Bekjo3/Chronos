#ifndef CHRONOS_FCFS_POLICY_H
#define CHRONOS_FCFS_POLICY_H

#include "scheduling_policy.h"
#include <string>
#include <vector>

namespace chronos {

class FCFSPolicy final : public ISchedulingPolicy {
public:
    ~FCFSPolicy() override = default;

    // Selects the job with the earliest arrival time.
    // Tie-breaker: smaller job_id.
    Job* getNextJob(std::vector<Job>& ready_queue) override;

    // FCFS has no special completion handling --> just moves on to the next one.
    void onJobCompletion(Job* /*completed_job*/, float /*current_time*/) override {}

    std::string getName() const override { return "FCFS"; }

    bool isPreemptive() const override { return false; }
};

}

#endif