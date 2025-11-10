#include "cli_parser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace chronos {

CLIOptions CLIParser::parse(int argc, char* argv[]) {
    CLIOptions options;

    if (argc < 2) {
        std::cerr << "Error: No arguments provided" << std::endl;
        std::cerr << "Use --help for usage information.\n";
        return options;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--algo" || arg == "-a") {
            if (i + 1 >= argc) { 
                std::cerr << "Error: --algo requires a value (FCFS, SJF, Priority, RR)\n";
                return options;
            }

            std::string algorithm_str = argv[++i];
            auto algorithm = stringToAlgorithm(algorithm_str);
            if (!algorithm.has_value()) {
                std::cerr << "Error: Invalid algorithm: '" << algorithm_str << 
                "' Must be one of FCFS, SJF, Priority, RR" << std::endl;
                return options;
            }

            options.algorithm = algorithm.value();

        }
        else if (arg == "--cores" || arg == "-c") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --cores requires a value\n";
                return options;
            }

            std::string value_str = argv[++i];
            try {
                int cores = std::stoi(value_str);
                if (cores <= 0) {
                    std::cerr << "Error: --cores must be a positive integer\n";
                    return options;
                }
                options.num_cores = cores;
            } catch (const std::exception& e) {
                std::cerr << "Error: --cores value '" << value_str 
                          << "' is not a valid integer\n";
                return options;
            }
        }
        else if (arg == "--jobs" || arg == "-j") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --jobs requires a value\n";
                return options;
            }

            std::string value_str = argv[++i];
            try {
                int jobs = std::stoi(value_str);
                if (jobs <= 0) {
                    std::cerr << "Error: --jobs must be a positive integer\n";
                    return options;
                }
                options.num_jobs = jobs;
            } catch (const std::exception& e) {
                std::cerr << "Error: --jobs value '" << value_str 
                          << "' is not a valid integer\n";
                return options;
            }
        }
        else if (arg == "--quantum" || arg == "-q") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --quantum requires a value\n";
                return options;
            }

            std::string value_str = argv[++i];
            try {
                int quantum_val = std::stoi(value_str);
                if (quantum_val <= 0) {
                    std::cerr << "Error: --quantum must be a positive integer\n";
                    return options;
                }
                options.quantum = quantum_val;
            } catch (const std::exception& e) {
                std::cerr << "Error: --quantum value '" << value_str 
                          << "' is not a valid integer\n";
                return options;
            }
        }
        else if (arg == "--compare-all" || arg == "--compare") {
            options.compare_all = true;
            options.algorithm = static_cast<SchedulingAlgorithm>(-1); // use sentinel value (-1) to represent "All" algorithms
        }
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: schedsim [OPTIONS]\n"
                      << "Options:\n"
                      << "  --algo, -a <ALGO>       Scheduling algorithm (FCFS, SJF, Priority, RR)\n"
                      << "  --cores, -c <NUM>       Number of CPU cores (positive integer)\n"
                      << "  --jobs, -j <NUM>        Number of jobs (positive integer)\n"
                      << "  --quantum, -q <NUM>     Time quantum for Round Robin (positive integer, optional)\n"
                      << "  --compare-all           Run all algorithms and compare results\n"
                      << "  --help, -h              Show this help message\n";
            options.is_valid = false;  // Help doesn't run the program
            return options;
        }
        else {
            std::cerr << "Error: Unknown argument: " << arg << std::endl;
            std::cout << "Use --help for usage information.\n";
            return options;
        }
    }

    if (validateOptions(options)) {
        options.is_valid = true;
    }
    return options;
}

void CLIParser::printOptions(const CLIOptions& options) {
    std::cout << "========================================\n";
    std::cout << "Parsed Command-Line Arguments:\n";
    std::cout << "========================================\n";

    if (!options.is_valid) {
        std::cout << "Status: INVALID (validation failed)\n";
        return;
    }
    
    std::cout << "Status: VALID\n";
    std::cout << "Algorithm: " << algorithmToString(options.algorithm) << "\n";
    std::cout << "CPU Cores: " << options.num_cores << "\n";
    std::cout << "Number of Jobs: " << options.num_jobs << "\n";
    
    if (options.quantum.has_value()) {
        std::cout << "Quantum: " << options.quantum.value() << "\n";
    } else {
        std::cout << "Quantum: Not specified";
        if (options.algorithm == SchedulingAlgorithm::RR) {
            std::cout << " (WARNING: Required for Round Robin)";
        }
        std::cout << "\n";
    }
    
    std::cout << "Compare All: " << (options.compare_all ? "Yes" : "No") << "\n";
    std::cout << "========================================\n";
}

std::string CLIParser::algorithmToString(const SchedulingAlgorithm& algo) {
    switch (algo) {
        case SchedulingAlgorithm::FCFS:     return "FCFS";
        case SchedulingAlgorithm::SJF:      return "SJF";
        case SchedulingAlgorithm::PRIORITY: return "Priority";
        case SchedulingAlgorithm::RR:       return "Round Robin";
        default:                            return "All";
    }
}

std::optional<SchedulingAlgorithm> CLIParser::stringToAlgorithm(const std::string& str) {
    // Convert to uppercase for case-insensitive matching
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    
    if (upper_str == "FCFS") {
        return SchedulingAlgorithm::FCFS;
    } else if (upper_str == "SJF") {
        return SchedulingAlgorithm::SJF;
    } else if (upper_str == "PRIORITY" || upper_str == "PRI") {
        return SchedulingAlgorithm::PRIORITY;
    } else if (upper_str == "RR" || upper_str == "ROUNDROBIN") {
        return SchedulingAlgorithm::RR;
    }
    
    return std::nullopt;
}

bool CLIParser::validateOptions(const CLIOptions& options) {
    // Validate that required fields are set
    if (options.num_cores <= 0) {
        std::cerr << "Error: --cores is required and must be positive\n";
        return false;
    }
    
    if (options.num_jobs <= 0) {
        std::cerr << "Error: --jobs is required and must be positive\n";
        return false;
    }
    
    // If Round Robin is selected, quantum should be specified
    if (options.algorithm == SchedulingAlgorithm::RR && !options.quantum.has_value()) {
        std::cerr << "Error: --quantum is required when using Round Robin (RR) algorithm\n";
        return false;
    }
    
    if (!options.compare_all && options.quantum.has_value() && options.algorithm != SchedulingAlgorithm::RR) {
        std::cout << "Warning: --quantum specified but algorithm is not Round Robin. "
                  << "Quantum will be ignored.\n";
    }
    
    return true;
}

}