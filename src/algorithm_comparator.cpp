#include "algorithm_comparator.h"

#include "fcfs_policy.h"
#include "sjf_policy.h"
#include "priority_policy.h"
#include "rr_policy.h"
#include "scheduler_engine.h"
#include "file_writer.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

namespace chronos {

std::unique_ptr<ISchedulingPolicy> AlgorithmComparator::createPolicy(
    SchedulingAlgorithm algo, int quantum) {
    switch (algo) {
        case SchedulingAlgorithm::FCFS:
            return std::make_unique<FCFSPolicy>();
        case SchedulingAlgorithm::SJF:
            return std::make_unique<SJFPolicy>();
        case SchedulingAlgorithm::PRIORITY:
            return std::make_unique<PriorityPolicy>();
        case SchedulingAlgorithm::RR:
            return std::make_unique<RoundRobinPolicy>(quantum);
        default:
            return nullptr;
    }
}

bool AlgorithmComparator::runAndWriteAlgorithm(
    std::vector<Job> jobs,
    std::unique_ptr<ISchedulingPolicy> policy,
    int num_cores,
    const std::string& output_dir) {
    
    if (!policy) {
        return false;
    }
    
    // Create a fresh copy of jobs for this algorithm run
    std::vector<Job> jobs_copy;
    jobs_copy.reserve(jobs.size());
    for (const auto& job : jobs) {
        // Reset job state for new run - create fresh job with same parameters
        Job new_job(job.getId(), job.getArrivalTime(), 
                   job.getBurstTime(), job.getPriority());
        jobs_copy.push_back(new_job);
    }
    
    // Run scheduler
    SchedulerEngine engine;
    auto schedule_result = engine.run(std::move(jobs_copy), *policy, num_cores);
    
    // Create metrics collector from schedule result
    MetricsCollector collector;
    collector.setMakespan(schedule_result.makespan);
    collector.setCpuActiveTime(schedule_result.cpu_active_time);
    collector.setContextSwitches(schedule_result.context_switches);
    collector.setNumCores(num_cores);
    for (const auto& job : schedule_result.completed_jobs) {
        collector.recordJobCompletion(job);
    }
    
    // Write to CSV (summary.csv will append, metrics.csv will overwrite)
    // For compare-all, we want to append metrics too, so we'll use a different approach
    return FileWriter::writeSummaryCSV(collector, *policy, output_dir + "/summary.csv");
}

bool AlgorithmComparator::compareAll(std::vector<Job> jobs, int num_cores, int quantum) {
    if (jobs.empty()) {
        std::cerr << "Error: No jobs provided for comparison\n";
        return false;
    }
    
    std::cout << "\n========================================\n";
    std::cout << "Running All Algorithms for Comparison\n";
    std::cout << "========================================\n\n";
    
    // List of algorithms to run
    const std::vector<SchedulingAlgorithm> algorithms = {
        SchedulingAlgorithm::FCFS,
        SchedulingAlgorithm::SJF,
        SchedulingAlgorithm::PRIORITY,
        SchedulingAlgorithm::RR
    };
    
    // Clear summary.csv for fresh comparison
    std::ofstream clear_file("output/summary.csv", std::ios::trunc);
    clear_file.close();
    
    bool all_success = true;
    
    // Run each algorithm
    for (auto algo : algorithms) {
        auto policy = createPolicy(algo, quantum);
        if (!policy) {
            std::cerr << "Error: Failed to create policy for " 
                      << CLIParser::algorithmToString(algo) << "\n";
            all_success = false;
            continue;
        }
        
        std::cout << "Running " << policy->getName() << "...\n";
        bool success = runAndWriteAlgorithm(jobs, std::move(policy), num_cores);
        if (!success) {
            std::cerr << "Warning: Failed to run " << policy->getName() << "\n";
            all_success = false;
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "Comparison complete!\n";
    std::cout << "Results written to output/summary.csv\n";
    std::cout << "========================================\n\n";
    
    return all_success;
}

} // namespace chronos