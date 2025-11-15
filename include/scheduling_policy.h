#ifndef CHRONOS_SCHEDULING_POLICY_H
#define CHRONOS_SCHEDULING_POLICY_H

#include "job.h"
#include <vector>
#include <memory>

namespace chronos {

// Defines the interface that all scheduling algorithms must implement (Abstract base class)
class ISchedulingPolicy {
public:
    virtual ~ISchedulingPolicy() = default;
    
 
    // Returns: pointer to the selected job, or nullptr if no job is available
    // Note: The policy should not remove the job from the queue; that's the scheduler's responsibility
    virtual Job* getNextJob(std::vector<Job>& ready_queue) = 0;
    

    // current_time: current simulation time
    // Note: For preemptive algorithms (like RR), this may need to re-queue the job
    virtual void onJobCompletion(Job* completed_job, float current_time) = 0;
    
    // Get the name of the scheduling policy (for logging/output)
    virtual std::string getName() const = 0;
    
    // Check if the policy is preemptive
    // Preemptive policies can interrupt running jobs
    virtual bool isPreemptive() const = 0;

    // Optional time slice (seconds). Negative ⇒ run job to completion.
    virtual float getTimeSlice() const { return -1.0f; }
};

}

#endif 