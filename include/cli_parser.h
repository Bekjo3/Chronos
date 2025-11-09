#ifndef CHRONOS_CLI_PARSER_H
#define CHRONOS_CLI_PARSER_H

#include <string>
#include <vector>
#include <optional>

namespace chronos {

// supported scheduling algorithms
enum class SchedulingAlgorithm {
    FCFS,        // First Come First Serve
    SJF,         // Shortest Job First
    PRIORITY,    // Priority Scheduling
    RR,          // Round Robin
};

struct CLIOptions {
    SchedulingAlgorithm algorithm = SchedulingAlgorithm::FCFS;
    int num_cores = 1;
    int num_jobs = 0;
    std::optional<int> quantum = 1;
    bool compare_all = false;

    bool is_valid = false;
};

class CLIParser {
public:
    static CLIOptions parse(int argc, char* argv[]);
    static void printOptions(const CLIOptions& options);
    static std::string algorithmToString(const SchedulingAlgorithm& algorithm);
    static std::optional<SchedulingAlgorithm> stringToAlgorithm(const std::string& str);

private:
    // Helper: Extract integer value from argument
    static bool parseIntArg(const std::string& arg, const std::string& flag, int& value);
    
    // Helper: Validate parsed options
    static bool validateOptions(const CLIOptions& options);

};

} // namespace chronos

#endif 