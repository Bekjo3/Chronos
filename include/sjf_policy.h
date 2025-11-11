#ifndef CHRONOS_SJF_POLICY_H
#define CHRONOS_SJF_POLICY_H

#include "scheduling_policy.h"
#include <string>
#include <vector>

namespace chronos {

// Shortest-Job-First policy (non-preemptive)
class SJFPolicy final : public ISchedulingPolicy {
public:
    ~SJFPolicy() override = default;

    // Pick job with the smallest burst time -->  tie-break by arrival, then job id.
    Job* getNextJob(std::vector<Job>& ready_queue) override;

    // SJF is non-preemptive
    void onJobCompletion(Job* /*completed_job*/, float /*current_time*/) override {}

    std::string getName() const override { return "SJF"; }

    bool isPreemptive() const override { return false; }
};

}

#endif