#ifndef CHRONOS_JOB_H
#define CHRONOS_JOB_H

#include <string>
#include <iostream>

namespace chronos {

// Job execution states
enum class JobState {
    NEW,        // Job is created but has not yet arrived
    READY,      // Job has arrived and is ready to be scheduled
    RUNNING,    // Job is currently executing on a CPU core
    WAITING,    // Job is waiting for I/O or other resources
    FINISHED    // Job has completed execution
};


class Job {
public:
    // Constructor
    Job(int id, float arrival_time, float burst_time, int priority = 0);
    

    int getId() const { return job_id; }
    float getArrivalTime() const { return arrival_time; }
    float getBurstTime() const { return burst_time; }
    float getRemainingTime() const { return remaining_time; }
    int getPriority() const { return priority; }
    JobState getState() const { return state; }
    float getStartTime() const { return start_time; }
    float getFinishTime() const { return finish_time; }
    float getWaitingTime() const { return waiting_time; }
    float getTurnaroundTime() const { return turnaround_time; }
    

    void setState(JobState new_state) { state = new_state; }
    void setStartTime(float time) { start_time = time; }
    void setFinishTime(float time) { finish_time = time; }
    void setRemainingTime(float time) { remaining_time = time; }
    
    // Update calculated metrics (waiting and turnaround times)
    void calculateMetrics();
    
    // Print job information to output stream
    void print(std::ostream& os = std::cout) const;
    
    // Print job in table format (for CLI output)
    void printTableRow(std::ostream& os = std::cout) const;
    
    // Comparison operators for sorting
    // Compare by arrival time (for FCFS)
    bool operator<(const Job& other) const;
    bool operator>(const Job& other) const;
    

    bool operator==(const Job& other) const;
    
    bool isFinished() const { return state == JobState::FINISHED; }
    
    bool hasArrived(float current_time) const { return arrival_time <= current_time; }

private:
    int job_id;              // Unique identifier
    float arrival_time;      // Time when job arrives in system
    float burst_time;        // Total CPU time required
    float remaining_time;    // Remaining CPU time (for preemptive algorithms)
    int priority;            // Priority value (higher = more priority)
    
    JobState state;          // Current execution state
    
    float start_time;        // Time when job starts execution
    float finish_time;       // Time when job completes
    
    // Calculated metrics
    float waiting_time;      // Time spent waiting (start_time - arrival_time)
    float turnaround_time;   // Total time in system (finish_time - arrival_time)
};

// Helper to convert JobState to string
std::string stateToString(JobState state);

} 

#endif