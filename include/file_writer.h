#ifndef CHRONOS_FILE_WRITER_H
#define CHRONOS_FILE_WRITER_H

#include "metrics_collector.h"
#include "scheduling_policy.h"

#include <string>

namespace chronos {

// Handles writing scheduling metrics to CSV files for visualization
class FileWriter {
public:
    // Write per-job metrics to metrics.csv
    // Returns true on success, false on error
    static bool writeMetricsCSV(const MetricsCollector& metrics,
                                const ISchedulingPolicy& policy,
                                const std::string& filename = "output/metrics.csv");
    
    // Write aggregate summary metrics to summary.csv
    static bool writeSummaryCSV(const MetricsCollector& metrics,
                                const ISchedulingPolicy& policy,
                                const std::string& filename = "output/summary.csv");
    
    // Write both metrics.csv and summary.csv
    static bool writeAll(const MetricsCollector& metrics,
                        const ISchedulingPolicy& policy,
                        const std::string& output_dir = "output");

private:
    // Helper to ensure output directory exists
    static bool ensureDirectoryExists(const std::string& dir_path);
};

}

#endif 