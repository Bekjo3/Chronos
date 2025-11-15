#ifndef CHRONOS_RR_POLICY_H
#define CHRONOS_RR_POLICY_H

#include "scheduling_policy.h"
#include <deque>
#include <string>
#include <vector>

namespace chronos {

// Round-Robin scheduling policy (preemptive).
// Jobs share CPU time in a fixed quantum and unfinished jobs are re-queued.
class RoundRobinPolicy final : public ISchedulingPolicy {
public:
    explicit RoundRobinPolicy(int quantum);
    ~RoundRobinPolicy() override = default;

    Job* getNextJob(std::vector<Job>& ready_queue) override;
    void onJobCompletion(Job* completed_job, float current_time) override;

    std::string getName() const override { return "Round Robin"; }
    bool isPreemptive() const override { return true; }
    float getTimeSlice() const override { return static_cast<float>(quantum_); }
    int getQuantum() const { return quantum_; }

private:
    int quantum_;
    std::deque<int> rotation_;                 // job ids in rotation order

    void syncRotationWithReadyQueue(const std::vector<Job>& ready_queue);
    bool containsJob(int job_id) const;
};

} 

#endif