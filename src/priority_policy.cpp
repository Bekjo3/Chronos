#include "priority_policy.h"
#include <algorithm>

namespace chronos {

Job* PriorityPolicy::getNextJob(std::vector<Job>& ready_queue) {
    if (ready_queue.empty()) {
        return nullptr;
    }

    auto it = std::max_element(
        ready_queue.begin(),
        ready_queue.end(),
        [](const Job& a, const Job& b) {
            if (a.getPriority() != b.getPriority()) {
                return a.getPriority() < b.getPriority();          
            }
            if (a.getArrivalTime() != b.getArrivalTime()) {
                return a.getArrivalTime() > b.getArrivalTime();    
            }
            return a.getId() > b.getId();                   
        }
    );

    return (it != ready_queue.end()) ? &(*it) : nullptr;
}

} 