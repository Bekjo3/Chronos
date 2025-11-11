#include "sjf_policy.h"
#include <algorithm>

namespace chronos {

Job* SJFPolicy::getNextJob(std::vector<Job>& ready_queue) {
    if (ready_queue.empty()) {
        return nullptr;
    }

    // Shortest burst first --> tie-break on arrival time, then job id.
    auto it = std::min_element(
        ready_queue.begin(),
        ready_queue.end(),
        [](const Job& a, const Job& b) {
            if (a.getBurstTime() != b.getBurstTime()) {
                return a.getBurstTime() < b.getBurstTime();
            }
            if (a.getArrivalTime() != b.getArrivalTime()) {
                return a.getArrivalTime() < b.getArrivalTime();
            }
            return a.getId() < b.getId();
        }
    );

    return (it != ready_queue.end()) ? &(*it) : nullptr;
}

} 