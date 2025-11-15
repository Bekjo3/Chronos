#include "file_writer.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace chronos {

bool FileWriter::writeMetricsCSV(const MetricsCollector& metrics,
                                const ISchedulingPolicy& policy,
                                const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << "\n";
        return false;
    }
    
    // Write CSV header
    file << "algorithm,job_id,arrival_time,burst_time,priority,"
         << "start_time,finish_time,waiting_time,turnaround_time,remaining_time\n";
    

    const auto& jobs = metrics.getCompletedJobs();
    const std::string algo_name = policy.getName();
    
    for (const auto& job : jobs) {
        file << algo_name << ","
             << job.getId() << ","
             << std::fixed << std::setprecision(2)
             << job.getArrivalTime() << ","
             << job.getBurstTime() << ","
             << job.getPriority() << ","
             << job.getStartTime() << ","
             << job.getFinishTime() << ","
             << job.getWaitingTime() << ","
             << job.getTurnaroundTime() << ","
             << job.getRemainingTime() << "\n";
    }
    
    file.close();
    if (file.fail()) {
        std::cerr << "Error: Failed to write to file: " << filename << "\n";
        return false;
    }
    
    return true;
}

bool FileWriter::writeSummaryCSV(const MetricsCollector& metrics,
                                 const ISchedulingPolicy& policy,
                                 const std::string& filename) {
    std::ofstream file(filename, std::ios::app); // Append mode for compare-all
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << "\n";
        return false;
    }
    
    // Check if file is empty (first write) and write header
    file.seekp(0, std::ios::end);
    bool is_empty = file.tellp() == 0;
    file.seekp(0, std::ios::beg);
    
    if (is_empty) {
        file << "algorithm,avg_waiting_time,avg_turnaround_time,"
             << "cpu_utilization,context_switches,num_jobs,makespan\n";
    }
    
    // Write aggregate metrics
    file << policy.getName() << ","
         << std::fixed << std::setprecision(2)
         << metrics.getAverageWaitingTime() << ","
         << metrics.getAverageTurnaroundTime() << ","
         << metrics.getCpuUtilizationPercent() << ","
         << metrics.getContextSwitches() << ","
         << metrics.getJobCount() << ","
         << metrics.getMakespan() << "\n";
    
    file.close();
    if (file.fail()) {
        std::cerr << "Error: Failed to write to file: " << filename << "\n";
        return false;
    }
    
    return true;
}

bool FileWriter::writeAll(const MetricsCollector& metrics,
                         const ISchedulingPolicy& policy,
                         const std::string& output_dir) {
    
    if (!ensureDirectoryExists(output_dir)) {
        std::cerr << "Warning: Could not create output directory: " << output_dir << "\n";
        // Continue anyway - might already exist
    }
    
    const std::string metrics_file = output_dir + "/metrics.csv";
    const std::string summary_file = output_dir + "/summary.csv";
    
    // For metrics.csv, we want to overwrite (single algorithm run)
    // For summary.csv, we append (to support compare-all mode)
    
    bool metrics_ok = writeMetricsCSV(metrics, policy, metrics_file);
    bool summary_ok = writeSummaryCSV(metrics, policy, summary_file);
    
    if (metrics_ok && summary_ok) {
        std::cout << "Metrics exported to:\n"
                  << "  - " << metrics_file << "\n"
                  << "  - " << summary_file << "\n";
        return true;
    }
    
    return false;
}

bool FileWriter::ensureDirectoryExists(const std::string& dir_path) {
    // Try to create directory
    // mkdir returns 0 on success, -1 on failure (directory may already exist)
    int result = mkdir(dir_path.c_str(), 0755);
    
    if (result == 0 || errno == EEXIST) {
        return true;
    }
    
    return false;
}

} // namespace chronos