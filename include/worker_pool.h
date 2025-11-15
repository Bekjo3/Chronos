#ifndef CHRONOS_WORKER_POOL_H
#define CHRONOS_WORKER_POOL_H

#include "job.h"
#include "scheduling_policy.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace chronos {

// Manages worker threads that simulate CPU cores executing jobs
class WorkerPool {
public:
    WorkerPool(int num_cores, ISchedulingPolicy& policy, 
               std::vector<Job>& ready_queue,
               std::mutex& queue_mutex,
               std::condition_variable& job_available,
               std::atomic<bool>& simulation_running,
               std::vector<Job>& completed_jobs,
               std::mutex& completed_mutex,
               std::atomic<size_t>& context_switches);
    
    ~WorkerPool();
    
    // Start all worker threads
    void start();
    
    // Stop all worker threads (wait for completion)
    void stop();
    
    // Get number of active workers
    int getNumCores() const { return num_cores_; }
    
    // Check if all workers are idle
    bool allIdle() const;

private:
    // Worker thread function - simulates CPU core execution
    void workerThread(int core_id);
    
    // Execute a job on a CPU core (simulated by sleeping)
    void executeJob(Job& job, float time_slice, int core_id);
    
    int num_cores_;
    ISchedulingPolicy& policy_;
    std::vector<Job>& ready_queue_;
    std::mutex& queue_mutex_;
    std::condition_variable& job_available_;
    std::atomic<bool>& simulation_running_;
    std::vector<Job>& completed_jobs_;
    std::mutex& completed_mutex_;
    std::atomic<size_t>& context_switches_;
    
    std::vector<std::thread> workers_;
    std::atomic<int> active_workers_;
    
    // Track which core is executing which job
    std::vector<std::atomic<Job*>> executing_jobs_;
};

}

#endif