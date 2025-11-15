#ifndef CHRONOS_ALGORITHM_COMPARATOR_H
#define CHRONOS_ALGORITHM_COMPARATOR_H

#include "job.h"
#include "scheduling_policy.h"
#include "cli_parser.h"

#include <memory>
#include <vector>

namespace chronos {

// Runs all scheduling algorithms on the same job set and compares results
class AlgorithmComparator {
public:
    // Run all algorithms (FCFS, SJF, Priority, RR) on the same job set
    static bool compareAll(std::vector<Job> jobs, int num_cores, int quantum = 2);
    
    // Print comparison summary table
    static void printComparisonSummary();

private:
    // Create a policy instance for the given algorithm
    static std::unique_ptr<ISchedulingPolicy> createPolicy(
        SchedulingAlgorithm algo, int quantum = 2);
    
    // Run a single algorithm and write results
    static bool runAndWriteAlgorithm(
        std::vector<Job> jobs,
        std::unique_ptr<ISchedulingPolicy> policy,
        int num_cores,
        const std::string& output_dir = "output");
};

} 

#endif 