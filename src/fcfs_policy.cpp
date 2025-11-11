#include "fcfs_policy.h"
#include <algorithm>

namespace chronos {

Job* FCFSPolicy::getNextJob(std::vector<Job>& ready_queue) {
    if (ready_queue.empty()) {
        return nullptr;
    }

    // Select by earliest arrival and tie-break on job_id.
    auto it = std::min_element(
        ready_queue.begin(),
        ready_queue.end(),
        [](const Job& a, const Job& b) {
            if (a.getArrivalTime() != b.getArrivalTime()) {
                return a.getArrivalTime() < b.getArrivalTime();
            }
            return a.getId() < b.getId();
        }
    );

    return (it != ready_queue.end()) ? &(*it) : nullptr;
}

}