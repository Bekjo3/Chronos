#include "rr_policy.h"

#include <algorithm>
#include <stdexcept>

namespace chronos {

namespace {
    bool jobInQueue(const std::vector<Job>& ready_queue, int job_id) {
        return std::any_of(
            ready_queue.begin(),
            ready_queue.end(),
            [job_id](const Job& job) { return job.getId() == job_id; });
    }

    Job* findJob(std::vector<Job>& ready_queue, int job_id) {
        auto it = std::find_if(
            ready_queue.begin(),
            ready_queue.end(),
            [job_id](const Job& job) { return job.getId() == job_id; });
        return (it != ready_queue.end()) ? &(*it) : nullptr;
    }
}

RoundRobinPolicy::RoundRobinPolicy(int quantum)
    : quantum_(quantum) {
    if (quantum_ <= 0) {
        throw std::invalid_argument("RoundRobinPolicy requires a positive quantum.");
    }
}

void RoundRobinPolicy::syncRotationWithReadyQueue(const std::vector<Job>& ready_queue) {
    // Drop any job ids that are no longer present in the ready queue.
    rotation_.erase(
        std::remove_if(
            rotation_.begin(),
            rotation_.end(),
            [&ready_queue](int job_id) { return !jobInQueue(ready_queue, job_id); }),
        rotation_.end());

    // append newly arrived jobs to the rotation.
    for (const Job& job : ready_queue) {
        if (!containsJob(job.getId())) {
            rotation_.push_back(job.getId());
        }
    }
}

bool RoundRobinPolicy::containsJob(int job_id) const {
    return std::find(rotation_.begin(), rotation_.end(), job_id) != rotation_.end();
}

Job* RoundRobinPolicy::getNextJob(std::vector<Job>& ready_queue) {
    if (ready_queue.empty()) {
        rotation_.clear();
        return nullptr;
    }

    syncRotationWithReadyQueue(ready_queue);

    while (!rotation_.empty()) {
        int next_id = rotation_.front();
        rotation_.pop_front();

        if (Job* job = findJob(ready_queue, next_id)) {
            return job;
        }
        
    }

    return nullptr;
}

void RoundRobinPolicy::onJobCompletion(Job* completed_job, float /*current_time*/) {
    if (completed_job == nullptr) {
        return;
    }

    // Remove any lingering occurrences of the job id (just defensive)
    rotation_.erase(
        std::remove(rotation_.begin(), rotation_.end(), completed_job->getId()),
        rotation_.end());

    // if the job still has remaining work --> requeue
    if (completed_job->getRemainingTime() > 0.0f) {
        rotation_.push_back(completed_job->getId());
    }
}

} 