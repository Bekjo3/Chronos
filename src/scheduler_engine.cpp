#include "scheduler_engine.h"
#include "worker_pool.h"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <thread>
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

ScheduleResult SchedulerEngine::run(std::vector<Job> jobs, ISchedulingPolicy& policy, int num_cores) {
    ScheduleResult result;

    if (jobs.empty()) {
        std::cout << "No jobs to schedule.\n";
        return result;
    }

    if (num_cores <= 0) {
        std::cerr << "Error: Number of cores must be positive\n";
        return result;
    }

    // Sort jobs by arrival time
    std::sort(jobs.begin(), jobs.end(), arrivalLess);
    const float simulation_start = jobs.front().getArrivalTime();

    // Shared data structures
    std::vector<Job> ready_queue;
    ready_queue.reserve(jobs.size());
    result.completed_jobs.reserve(jobs.size());
    
    // Shared storage for completed jobs (thread-safe)
    std::vector<Job> completed_jobs_shared;
    completed_jobs_shared.reserve(jobs.size());
    std::mutex completed_mutex;
    
    std::mutex queue_mutex;
    std::condition_variable job_available;
    std::atomic<bool> simulation_running(true);
    std::atomic<float> current_time(simulation_start);

    // Create and start worker pool
    std::atomic<size_t> context_switch_counter(0);
    WorkerPool worker_pool(num_cores, policy, ready_queue, 
                          queue_mutex, job_available, simulation_running,
                          completed_jobs_shared, completed_mutex, context_switch_counter);
    worker_pool.start();

    // Start scheduler thread
    std::thread scheduler(&SchedulerEngine::schedulerThread, this,
                         std::move(jobs), std::ref(policy),
                         std::ref(ready_queue), std::ref(queue_mutex),
                         std::ref(job_available), std::ref(simulation_running),
                         std::ref(result), std::ref(worker_pool));

    // Wait for scheduler to finish
    scheduler.join();

    // Stop worker threads
    simulation_running.store(false);
    job_available.notify_all();
    worker_pool.stop();

    // Collect completed jobs from shared storage
    {
        std::lock_guard<std::mutex> lock(completed_mutex);
        result.completed_jobs = std::move(completed_jobs_shared);
    }
    
    // Collect any remaining completed jobs from ready queue
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        collectCompletedJobs(ready_queue, result.completed_jobs, queue_mutex);
    }

    // Calculate final metrics from actual job completion times
    float earliest_start = simulation_start;
    float latest_finish = simulation_start;
    
    for (const auto& job : result.completed_jobs) {
        result.total_waiting_time += job.getWaitingTime();
        result.total_turnaround_time += job.getTurnaroundTime();
        result.cpu_active_time += job.getBurstTime();
        
        if (job.getStartTime() >= 0.0f && (earliest_start == simulation_start || job.getStartTime() < earliest_start)) {
            earliest_start = job.getStartTime();
        }
        if (job.getFinishTime() > latest_finish) {
            latest_finish = job.getFinishTime();
        }
    }
    
    // Makespan = time from first job start to last job finish
    result.makespan = latest_finish - earliest_start;
    if (result.makespan < EPSILON) {
        result.makespan = 0.0f;
    }
    
    result.num_cores = num_cores;
    
    result.context_switches = context_switch_counter.load();
    result.dispatch_count = result.completed_jobs.size();

    printSummary(result, policy);
    return result;
}

void SchedulerEngine::schedulerThread(std::vector<Job> jobs, ISchedulingPolicy& policy,
                                     std::vector<Job>& ready_queue,
                                     std::mutex& queue_mutex,
                                     std::condition_variable& job_available,
                                     std::atomic<bool>& simulation_running,
                                     ScheduleResult& result,
                                     WorkerPool& worker_pool) {
    std::deque<Job> pending;
    pending.insert(pending.end(),
                   std::make_move_iterator(jobs.begin()),
                   std::make_move_iterator(jobs.end()));

    float current_time = jobs.empty() ? 0.0f : jobs.front().getArrivalTime();
    const float simulation_start = current_time;

    while (true) {
        // Admit newly arrived jobs to ready queue
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            while (!pending.empty() && pending.front().getArrivalTime() <= current_time + EPSILON) {
                Job job = std::move(pending.front());
                pending.pop_front();
                job.setState(JobState::READY);
                ready_queue.push_back(std::move(job));
                job_available.notify_one(); // Notify worker threads
            }
        }

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (pending.empty() && ready_queue.empty() && worker_pool.allIdle()) {
                break;
            }
        }

        // Advance time if no jobs are ready
        if (ready_queue.empty() && !pending.empty()) {
            const float next_arrival = pending.front().getArrivalTime();
            if (next_arrival > current_time) {
                result.idle_time += next_arrival - current_time;
                current_time = next_arrival;
            }
        }

        // Small sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Mark simulation as complete
    simulation_running.store(false);
    job_available.notify_all();
}

void SchedulerEngine::collectCompletedJobs(std::vector<Job>& ready_queue,
                                          std::vector<Job>& completed_jobs,
                                          std::mutex& queue_mutex) {
    // Separate completed jobs from ready jobs
    std::vector<Job> still_ready;
    for (auto& job : ready_queue) {
        if (job.getState() == JobState::FINISHED) {
            completed_jobs.push_back(std::move(job));
        } else {
            still_ready.push_back(std::move(job));
        }
    }
    ready_queue = std::move(still_ready);
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