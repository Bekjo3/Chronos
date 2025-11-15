#include "worker_pool.h"

#include <chrono>
#include <iostream>

namespace chronos {

WorkerPool::WorkerPool(int num_cores, ISchedulingPolicy& policy,
                       std::vector<Job>& ready_queue,
                       std::mutex& queue_mutex,
                       std::condition_variable& job_available,
                       std::atomic<bool>& simulation_running)
    : num_cores_(num_cores)
    , policy_(policy)
    , ready_queue_(ready_queue)
    , queue_mutex_(queue_mutex)
    , job_available_(job_available)
    , simulation_running_(simulation_running)
    , active_workers_(0)
    , current_time_(0.0f)
    , executing_jobs_(num_cores)
{
    for (int i = 0; i < num_cores_; ++i) {
        executing_jobs_[i].store(nullptr);
    }
}

WorkerPool::~WorkerPool() {
    stop();
}

void WorkerPool::start() {
    workers_.reserve(num_cores_);
    for (int i = 0; i < num_cores_; ++i) {
        workers_.emplace_back(&WorkerPool::workerThread, this, i);
    }
}

void WorkerPool::stop() {
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
}

bool WorkerPool::allIdle() const {
    return active_workers_.load() == 0;
}

void WorkerPool::workerThread(int core_id) {
    while (simulation_running_.load() || !ready_queue_.empty()) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Wait for jobs to be available or simulation to end
        job_available_.wait(lock, [this] {
            return !ready_queue_.empty() || !simulation_running_.load();
        });
        
        // Check if we should exit
        if (!simulation_running_.load() && ready_queue_.empty()) {
            break;
        }
        
        // Get next job from policy
        Job* selected_job = policy_.getNextJob(ready_queue_);
        if (!selected_job) {
            continue;
        }
        
        // Find job in ready queue and mark as running
        auto it = std::find_if(ready_queue_.begin(), ready_queue_.end(),
            [selected_job](const Job& job) {
                return job.getId() == selected_job->getId();
            });
        
        if (it == ready_queue_.end()) {
            continue;
        }
        
        Job& job = *it;
        
        // Set job state and timestamps
        const float dispatch_time = std::max(current_time_.load(), job.getArrivalTime());
        if (job.getStartTime() < 0.0f) {
            job.setStartTime(dispatch_time);
        }
        current_time_.store(dispatch_time);
        
        job.setState(JobState::RUNNING);
        executing_jobs_[core_id].store(&job);
        active_workers_.fetch_add(1);
        
        // Get execution time slice
        const float time_slice = policy_.getTimeSlice();
        const float remaining = job.getRemainingTime();
        float execution = remaining;
        if (time_slice > 0.0f) {
            execution = std::min(remaining, time_slice);
        }
        if (execution < 0.001f) {
            execution = remaining;
        }
        
        // Remove job from ready queue before releasing lock
        ready_queue_.erase(it);
        lock.unlock();
        
        // Execute job (simulate CPU execution by sleeping)
        executeJob(job, execution, core_id);
        
        // Update time and remaining work
        const float new_time = current_time_.load() + execution;
        current_time_.store(new_time);
        
        float new_remaining = remaining - execution;
        if (new_remaining < 0.001f) {
            new_remaining = 0.0f;
        }
        job.setRemainingTime(new_remaining);
        
        // Re-acquire lock for completion handling
        lock.lock();
        
        if (new_remaining <= 0.001f) {
            // Job completed
            job.setRemainingTime(0.0f);
            job.setFinishTime(new_time);
            job.setState(JobState::FINISHED);
            job.calculateMetrics();
            policy_.onJobCompletion(&job, new_time);
        } else {
            // Job not finished, put back in ready queue
            job.setState(JobState::READY);
            ready_queue_.push_back(job);
            policy_.onJobCompletion(&job, new_time);
            job_available_.notify_one(); // Notify other workers
        }
        
        executing_jobs_[core_id].store(nullptr);
        active_workers_.fetch_sub(1);
    }
}

void WorkerPool::executeJob(Job& job, float time_slice, int core_id) {
    // Simulate CPU execution by sleeping proportional to burst time
    // For simulation, we use milliseconds (1 second = 1000ms)
    const int sleep_ms = static_cast<int>(time_slice * 1000.0f);
    if (sleep_ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
   
}

}