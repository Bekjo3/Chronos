#include "cli_parser.h"
#include "job.h"
#include "scheduler_engine.h"
#include "algorithm_comparator.h"
#include "file_writer.h"
#include "metrics_collector.h"
#include "fcfs_policy.h"
#include "sjf_policy.h"
#include "priority_policy.h"
#include "rr_policy.h"
#include "scheduling_policy.h"

#include <iostream>
#include <memory>
#include <random>
#include <vector>

namespace chronos {

// Generate sample jobs for testing
std::vector<Job> generateSampleJobs(int num_jobs) {
    std::vector<Job> jobs;
    jobs.reserve(num_jobs);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> arrival_dist(0.0f, 10.0f);
    std::uniform_real_distribution<float> burst_dist(1.0f, 10.0f);
    std::uniform_int_distribution<int> priority_dist(1, 5);
    
    for (int i = 1; i <= num_jobs; ++i) {
        float arrival = arrival_dist(gen);
        float burst = burst_dist(gen);
        int priority = priority_dist(gen);
        jobs.emplace_back(i, arrival, burst, priority);
    }
    
    return jobs;
}

// Create policy from CLI options
std::unique_ptr<ISchedulingPolicy> createPolicy(const CLIOptions& options) {
    switch (options.algorithm) {
        case SchedulingAlgorithm::FCFS:
            return std::make_unique<FCFSPolicy>();
        case SchedulingAlgorithm::SJF:
            return std::make_unique<SJFPolicy>();
        case SchedulingAlgorithm::PRIORITY:
            return std::make_unique<PriorityPolicy>();
        case SchedulingAlgorithm::RR:
            if (options.quantum.has_value()) {
                return std::make_unique<RoundRobinPolicy>(options.quantum.value());
            } else {
                std::cerr << "Error: Quantum required for Round Robin algorithm\n";
                return nullptr;
            }
        default:
            return nullptr;
    }
}

} // namespace chronos

int main(int argc, char* argv[]) {
    using namespace chronos;
    
    auto options = CLIParser::parse(argc, argv);
    
    if (!options.is_valid) {
        return 1;
    }
    
    // Generate jobs
    if (options.num_jobs <= 0) {
        std::cerr << "Error: Number of jobs must be positive\n";
        return 1;
    }
    
    std::vector<Job> jobs = generateSampleJobs(options.num_jobs);
    
    if (options.compare_all) {
        int quantum = options.quantum.value_or(2);
        bool success = AlgorithmComparator::compareAll(
            jobs, options.num_cores, quantum);
        return success ? 0 : 1;
    }
    
    auto policy = createPolicy(options);
    if (!policy) {
        std::cerr << "Error: Failed to create scheduling policy\n";
        return 1;
    }
    
    // Run scheduler
    SchedulerEngine engine;
    auto result = engine.run(std::move(jobs), *policy, options.num_cores);
    
    MetricsCollector collector;
    collector.setMakespan(result.makespan);
    collector.setCpuActiveTime(result.cpu_active_time);
    collector.setContextSwitches(result.context_switches);
    collector.setNumCores(options.num_cores);
    for (const auto& job : result.completed_jobs) {
        collector.recordJobCompletion(job);
    }
    
    FileWriter::writeAll(collector, *policy, "output");
    
    return 0;
}
