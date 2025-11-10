#include "job.h"
#include <iomanip>
#include <sstream>

namespace chronos {

Job::Job(int id, float arrival_time, float burst_time, int priority)
    : job_id(id)
    , arrival_time(arrival_time)
    , burst_time(burst_time)
    , remaining_time(burst_time)
    , priority(priority)
    , state(JobState::NEW)
    , start_time(-1.0f)
    , finish_time(-1.0f)
    , waiting_time(0.0f)
    , turnaround_time(0.0f)
{
}

void Job::calculateMetrics() {
    if (start_time >= 0.0f && arrival_time >= 0.0f) {
        waiting_time = start_time - arrival_time;
        if (waiting_time < 0.0f) {
            waiting_time = 0.0f;  // Job can't wait negative time
        }
    }
    
    if (finish_time >= 0.0f && arrival_time >= 0.0f) {
        turnaround_time = finish_time - arrival_time;
        if (turnaround_time < 0.0f) {
            turnaround_time = 0.0f;
        }
    }
}

void Job::print(std::ostream& os) const {
    os << "Job " << job_id << ":\n";
    os << "  Arrival Time: " << arrival_time << "\n";
    os << "  Burst Time: " << burst_time << "\n";
    os << "  Remaining Time: " << remaining_time << "\n";
    os << "  Priority: " << priority << "\n";
    os << "  State: " << stateToString(state) << "\n";
    
    if (start_time >= 0.0f) {
        os << "  Start Time: " << start_time << "\n";
    }
    if (finish_time >= 0.0f) {
        os << "  Finish Time: " << finish_time << "\n";
    }
    if (waiting_time >= 0.0f) {
        os << "  Waiting Time: " << waiting_time << "\n";
    }
    if (turnaround_time >= 0.0f) {
        os << "  Turnaround Time: " << turnaround_time << "\n";
    }
}

void Job::printTableRow(std::ostream& os) const {
    // Format: Job | Arrival | Burst | Start | Finish | Wait | Turnaround
    os << std::setw(4) << job_id << " | "
       << std::setw(7) << std::fixed << std::setprecision(1) << arrival_time << " | "
       << std::setw(5) << burst_time << " | "
       << std::setw(5);
    
    if (start_time >= 0.0f) {
        os << start_time;
    } else {
        os << "N/A";
    }
    
    os << " | " << std::setw(6);
    
    if (finish_time >= 0.0f) {
        os << finish_time;
    } else {
        os << "N/A";
    }
    
    os << " | " << std::setw(4);
    
    if (waiting_time >= 0.0f) {
        os << std::setprecision(1) << waiting_time;
    } else {
        os << "N/A";
    }
    
    os << " | " << std::setw(10);
    
    if (turnaround_time >= 0.0f) {
        os << turnaround_time;
    } else {
        os << "N/A";
    }
    
    os << "\n";
}

bool Job::operator<(const Job& other) const {
    // Default comparison: by arrival time (for FCFS)
    if (arrival_time != other.arrival_time) {
        return arrival_time < other.arrival_time;
    }
    // Tie-break: by job ID
    return job_id < other.job_id;
}

bool Job::operator>(const Job& other) const {
    return other < *this;
}

bool Job::operator==(const Job& other) const {
    return job_id == other.job_id;
}

std::string stateToString(JobState state) {
    switch (state) {
        case JobState::NEW:      return "NEW";
        case JobState::READY:     return "READY";
        case JobState::RUNNING:   return "RUNNING";
        case JobState::WAITING:   return "WAITING";
        case JobState::FINISHED:  return "FINISHED";
        default:                  return "UNKNOWN";
    }
}

} 