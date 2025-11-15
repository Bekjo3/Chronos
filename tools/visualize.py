#!/usr/bin/env python3
"""
Generates visualizations from scheduler metrics CSV files:
- Gantt chart showing job execution timeline
- Bar chart comparing average metrics across algorithms
- CPU utilization and context switches comparison chart
"""

import csv
import os
import sys
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
from collections import defaultdict
from typing import Dict, List, Tuple


def read_metrics_csv(filename: str) -> List[Dict]:
    metrics = []
    if not os.path.exists(filename):
        print(f"Warning: {filename} not found. Skipping Gantt chart.")
        return metrics
    
    try:
        with open(filename, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                try:
                    metrics.append({
                        'algorithm': row['algorithm'],
                        'job_id': int(row['job_id']),
                        'arrival_time': float(row['arrival_time']),
                        'burst_time': float(row['burst_time']),
                        'start_time': float(row['start_time']),
                        'finish_time': float(row['finish_time']),
                        'waiting_time': float(row['waiting_time']),
                        'turnaround_time': float(row['turnaround_time']),
                    })
                except (KeyError, ValueError) as e:
                    print(f"Warning: Skipping malformed row in {filename}: {e}")
                    continue
        
        if not metrics:
            print(f"Warning: {filename} is empty or contains no valid data.")
        
    except Exception as e:
        print(f"Error reading {filename}: {e}")
        return []
    
    return metrics


def read_summary_csv(filename: str) -> List[Dict]:
    summary = []
    if not os.path.exists(filename):
        print(f"Warning: {filename} not found. Skipping comparison charts.")
        return summary
    
    try:
        with open(filename, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                try:
                    summary.append({
                        'algorithm': row['algorithm'],
                        'avg_waiting_time': float(row['avg_waiting_time']),
                        'avg_turnaround_time': float(row['avg_turnaround_time']),
                        'cpu_utilization': float(row['cpu_utilization']),
                        'context_switches': int(row['context_switches']),
                        'num_jobs': int(row['num_jobs']),
                        'makespan': float(row['makespan']),
                    })
                except (KeyError, ValueError) as e:
                    print(f"Warning: Skipping malformed row in {filename}: {e}")
                    continue
        
        if not summary:
            print(f"Warning: {filename} is empty or contains no valid data.")
            
    except Exception as e:
        print(f"Error reading {filename}: {e}")
        return []
    
    return summary


def generate_gantt_chart(metrics: List[Dict], output_file: str = 'output/gantt_chart.png'):
    if not metrics:
        print("No metrics data available for Gantt chart.")
        return
    

    algorithm = metrics[0]['algorithm']
    
    sorted_jobs = sorted(metrics, key=lambda x: (x['start_time'], x['job_id']))
    
    fig, ax = plt.subplots(figsize=(12, max(6, len(sorted_jobs) * 0.5)))
    
    # Generate color map for jobs
    job_ids = sorted(set(job['job_id'] for job in sorted_jobs))
    colors = plt.cm.tab20(np.linspace(0, 1, len(job_ids)))
    job_color_map = {job_id: colors[i] for i, job_id in enumerate(job_ids)}
    
    # Plot each job as a horizontal bar
    y_positions = []
    labels = []
    max_time = 0
    
    for i, job in enumerate(sorted_jobs):
        y_pos = i
        y_positions.append(y_pos)
        labels.append(f"Job {job['job_id']}")
        
        start = job['start_time']
        duration = job['finish_time'] - job['start_time']
        job_id = job['job_id']
        
        # Draw execution bar
        legend = ax.get_legend()
        existing_labels = []

        if legend:
            existing_labels = [l.get_text().split()[1] for l in legend.get_texts()]

        if job_id not in existing_labels:
            label = f"Job {job_id}"
        else:
            label = None

        ax.barh(y_pos, duration, left=start,
                color=job_color_map[job_id],
                edgecolor='black', linewidth=1, alpha=0.7,
                label=label)
        
        # Draw waiting period (from arrival to start) in lighter color
        if job['waiting_time'] > 0.01:
            ax.barh(y_pos, job['waiting_time'], left=job['arrival_time'],
                   color=job_color_map[job_id], edgecolor='black',
                   linewidth=1, alpha=0.3, hatch='///')
        
        max_time = max(max_time, job['finish_time'])
    
    # Customize plot
    ax.set_yticks(y_positions)
    ax.set_yticklabels(labels)
    ax.set_xlabel('Time (seconds)', fontsize=12)
    ax.set_ylabel('Jobs', fontsize=12)
    ax.set_title(f'Gantt Chart - {algorithm} Algorithm\n'
                f'(Execution: solid, Waiting: hatched)', fontsize=14, fontweight='bold')
    ax.grid(True, axis='x', linestyle='--', alpha=0.3)
    ax.set_xlim(left=0)
    
    # Create legend for job colors (limit to avoid clutter)
    if len(job_ids) <= 20:
        handles = [mpatches.Patch(color=job_color_map[jid], label=f'Job {jid}') 
                  for jid in sorted(job_ids)]
        ax.legend(handles=handles, loc='upper right', ncol=min(4, len(job_ids)), 
                 fontsize=8, title='Job IDs')
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"Gantt chart saved to {output_file}")
    plt.close()


def generate_metrics_bar_chart(summary: List[Dict], output_file: str = 'output/avg_metrics.png'):
    if not summary:
        print("No summary data available for bar chart.")
        return
    
    algorithms = [s['algorithm'] for s in summary]
    avg_wait = [s['avg_waiting_time'] for s in summary]
    avg_turnaround = [s['avg_turnaround_time'] for s in summary]
    
    x = np.arange(len(algorithms))
    width = 0.35
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    bars1 = ax.bar(x - width/2, avg_wait, width, label='Average Waiting Time', 
                   color='skyblue', edgecolor='black', linewidth=1)
    bars2 = ax.bar(x + width/2, avg_turnaround, width, label='Average Turnaround Time',
                   color='lightcoral', edgecolor='black', linewidth=1)
    
    # Add value labels on bars
    def autolabel(bars):
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height,
                   f'{height:.2f}',
                   ha='center', va='bottom', fontsize=9)
    
    autolabel(bars1)
    autolabel(bars2)
    
    ax.set_ylabel('Average Time (seconds)', fontsize=12)
    ax.set_title('Average Metrics by Scheduling Algorithm', fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(algorithms)
    ax.legend(fontsize=11)
    ax.grid(True, axis='y', linestyle='--', alpha=0.6)
    ax.set_ylim(bottom=0)
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"Average metrics chart saved to {output_file}")
    plt.close()


def generate_utilization_chart(summary: List[Dict], output_file: str = 'output/utilization.png'):
    if not summary:
        print("No summary data available for utilization chart.")
        return
    
    algorithms = [s['algorithm'] for s in summary]
    utilization = [s['cpu_utilization'] for s in summary]
    context_switches = [s['context_switches'] for s in summary]
    
    fig, ax1 = plt.subplots(figsize=(10, 6))
    
    # Bar chart for CPU utilization
    bars = ax1.bar(algorithms, utilization, color='skyblue', edgecolor='black', 
                   linewidth=1, alpha=0.7, label='CPU Utilization')
    
    # Add value labels on utilization bars
    for i, (bar, util) in enumerate(zip(bars, utilization)):
        height = bar.get_height()
        ax1.text(bar.get_x() + bar.get_width()/2., height,
                f'{util:.1f}%',
                ha='center', va='bottom', fontsize=10, fontweight='bold')
    
    ax1.set_xlabel('Algorithm', fontsize=12)
    ax1.set_ylabel('CPU Utilization (%)', fontsize=12, color='blue')
    ax1.tick_params(axis='y', labelcolor='blue')
    ax1.set_ylim(0, 105)
    ax1.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    # Line plot for context switches on secondary axis
    ax2 = ax1.twinx()
    line = ax2.plot(algorithms, context_switches, 'ro-', linewidth=2, 
                   markersize=10, label='Context Switches', color='red')
    ax2.set_ylabel('Context Switches', fontsize=12, color='red')
    ax2.tick_params(axis='y', labelcolor='red')
    
    # Add value labels for context switches
    for i, (algo, switches) in enumerate(zip(algorithms, context_switches)):
        ax2.text(i, switches, f' {switches}',
                ha='left', va='bottom', fontsize=10, color='red', fontweight='bold')
    
    # Combine legends
    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left', fontsize=11)
    
    ax1.set_title('CPU Utilization vs Context Switches', fontsize=14, fontweight='bold')
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"Utilization chart saved to {output_file}")
    plt.close()


def main():
    """Main function to generate all visualizations"""
    # Default paths
    output_dir = 'output'
    metrics_file = os.path.join(output_dir, 'metrics.csv')
    summary_file = os.path.join(output_dir, 'summary.csv')
    
    # Allow command-line override
    if len(sys.argv) > 1:
        output_dir = sys.argv[1]
        metrics_file = os.path.join(output_dir, 'metrics.csv')
        summary_file = os.path.join(output_dir, 'summary.csv')
    
    print("Chronos Visualization Tool")
    print("=" * 50)
    print(f"Reading data from: {output_dir}/")
    print()
    
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    # Read data
    metrics = read_metrics_csv(metrics_file)
    summary = read_summary_csv(summary_file)
    
    if not metrics and not summary:
        print("\nError: No valid data found in CSV files.")
        print("Please run the scheduler first to generate data:")
        print("  ./schedsim --cores 2 --algo FCFS --jobs 5")
        print("or:")
        print("  ./schedsim --cores 2 --jobs 10 --compare-all")
        sys.exit(1)
    
    print("Generating visualizations...")
    print()
    
    if metrics:
        generate_gantt_chart(metrics, os.path.join(output_dir, 'gantt_chart.png'))
    
    if summary:
        generate_metrics_bar_chart(summary, os.path.join(output_dir, 'avg_metrics.png'))
        generate_utilization_chart(summary, os.path.join(output_dir, 'utilization.png'))
    
    print()
    print("=" * 50)
    print("Visualization complete!")
    print(f"Charts saved to: {output_dir}/")


if __name__ == '__main__':
    main()

