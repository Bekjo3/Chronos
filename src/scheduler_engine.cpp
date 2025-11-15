#include "scheduler_engine.h"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <vector>

namespace chronos {
namespace {
constexpr float EPSILON = 1e-5f;

bool arrivalLess(const Job& lhs, const Job& rhs) {
    if (lhs.getArrivalTime() != rhs.getArrivalTime()) {
        return lhs.getArrivalTime() < rhs.getArrivalTime();
    }
    return lhs.getId() < rhs.getId();
}
} // namespace

ScheduleResult SchedulerEngine::run(std::vector<Job> jobs, ISchedulingPolicy& policy) {
    ScheduleResult result;

    if (jobs.empty()) {
        std::cout << "No jobs to schedule.\n";
        return result;
    }

    std::sort(jobs.begin(), jobs.end(), arrivalLess);
    const float simulation_start = jobs.front().getArrivalTime();

    std::deque<Job> pending;
    pending.insert(pending.end(),
                   std::make_move_iterator(jobs.begin()),
                   std::make_move_iterator(jobs.end()));

    std::vector<Job> ready_queue;
    ready_queue.reserve(pending.size());
    result.completed_jobs.reserve(pending.size());

    float current_time = simulation_start;

    while (!pending.empty() || !ready_queue.empty()) {
        // Admit newly arrived jobs.
        while (!pending.empty() && pending.front().getArrivalTime() <= current_time + EPSILON) {
            Job job = std::move(pending.front());
            pending.pop_front();
            job.setState(JobState::READY);
            ready_queue.push_back(std::move(job));
        }

        if (ready_queue.empty()) {
            if (!pending.empty()) {
                const float next_arrival = pending.front().getArrivalTime();
                if (next_arrival > current_time) {
                    result.idle_time += next_arrival - current_time;
                    current_time = next_arrival;
                }
            }
            continue;
        }

        Job* selected = policy.getNextJob(ready_queue);
        if (!selected) {
            selected = &ready_queue.front();
        }

        const std::size_t idx = static_cast<std::size_t>(selected - ready_queue.data());
        Job& job = ready_queue[idx];

        const float dispatch_time = std::max(current_time, job.getArrivalTime());
        if (job.getStartTime() < 0.0f) {
            job.setStartTime(dispatch_time);
        }
        if (dispatch_time - current_time > EPSILON) {
            result.idle_time += dispatch_time - current_time;
            current_time = dispatch_time;
        }

        job.setState(JobState::RUNNING);
        result.dispatch_count++;

        const float time_slice = policy.getTimeSlice();
        const float remaining = job.getRemainingTime();
        float execution = remaining;
        if (time_slice > 0.0f) {
            execution = std::min(remaining, time_slice);
        }
        if (execution < EPSILON) {
            execution = remaining;
        }

        current_time += execution;
        result.cpu_active_time += execution;

        float new_remaining = remaining - execution;
        if (new_remaining < EPSILON) {
            new_remaining = 0.0f;
        }
        job.setRemainingTime(new_remaining);

        if (new_remaining <= EPSILON) {
            job.setRemainingTime(0.0f);
            job.setFinishTime(current_time);
            job.setState(JobState::FINISHED);
            job.calculateMetrics();

            const float waiting = job.getWaitingTime();
            const float turnaround = job.getTurnaroundTime();

            policy.onJobCompletion(&job, current_time);

            result.total_waiting_time += waiting;
            result.total_turnaround_time += turnaround;

            result.completed_jobs.push_back(std::move(job));
            ready_queue.erase(ready_queue.begin() + static_cast<std::ptrdiff_t>(idx));
        } else {
            job.setState(JobState::READY);
            policy.onJobCompletion(&job, current_time);
        }

        result.makespan = current_time - simulation_start;
    }

    if (result.makespan < EPSILON) {
        result.makespan = 0.0f;
    }

    printSummary(result, policy);
    return result;
}

void SchedulerEngine::printSummary(const ScheduleResult& result, const ISchedulingPolicy& policy) const {
    const auto original_flags = std::cout.flags();
    const auto original_precision = std::cout.precision();

    std::cout << "Algorithm: " << policy.getName();
    const float slice = policy.getTimeSlice();
    if (slice > 0.0f) {
        std::cout << " (Quantum = " << slice << ")";
    }
    std::cout << "\n";
    std::cout << "------------------------------------------------\n";
    std::cout << "Job | Arrival | Burst | Start | Finish | Wait | Turnaround\n";
    std::cout << "------------------------------------------------\n";
    printJobTable(result.completed_jobs);
    std::cout << "------------------------------------------------\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average Waiting Time: " << result.averageWaitingTime() << "\n";
    std::cout << "Average Turnaround Time: " << result.averageTurnaroundTime() << "\n";
    std::cout << "CPU Utilization: " << result.cpuUtilization() * 100.0f << "%\n";
    std::cout << "Context Switches: " << result.contextSwitches() << "\n";

    std::cout.flags(original_flags);
    std::cout.precision(original_precision);
}

void SchedulerEngine::printJobTable(const std::vector<Job>& jobs) const {
    std::vector<const Job*> ordered;
    ordered.reserve(jobs.size());
    for (const auto& job : jobs) {
        ordered.push_back(&job);
    }

    std::sort(ordered.begin(), ordered.end(),
              [](const Job* lhs, const Job* rhs) { return lhs->getId() < rhs->getId(); });

    for (const Job* job : ordered) {
        job->printTableRow();
    }
}

}