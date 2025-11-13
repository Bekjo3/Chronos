#ifndef CHRONOS_PRIORITY_POLICY_H
#define CHRONOS_PRIORITY_POLICY_H

#include "scheduling_policy.h"
#include <string>
#include <vector>

namespace chronos {

// Priority-based scheduling policy (non-preemptive).
// Higher numeric priority wins and ties are broken by arrival time, then job id.
class PriorityPolicy final : public ISchedulingPolicy {
public:
    ~PriorityPolicy() override = default;

    Job* getNextJob(std::vector<Job>& ready_queue) override;

    // Non-preemptive
    void onJobCompletion(Job* , float /*current_time*/) override {}

    std::string getName() const override { return "Priority"; }

    bool isPreemptive() const override { return false; }
};

} 

#endif 