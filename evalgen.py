# TODO: Coherence_eviction: Change x-axis to workload instead of proto
# TODO: Directory traffic: Fix to a specific number of processors
# TODO: With and without locality tracegen
# TODO: Change the scalability plot for directory and snooping to show better difference.
# TODO: Instead of avg, make it more specific
# TODO: Hit, miss, evict for each proto per workload

import subprocess
import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from itertools import product
import argparse
from pathlib import Path
import json

# The style can be changed but I'm following the seaborn style
sns.set_theme(style="whitegrid")
plt.rcParams.update({
    'figure.figsize': (14, 10),  
    'font.size': 14,
    'axes.titlesize': 18,
    'axes.labelsize': 16,
    'xtick.labelsize': 14,
    'ytick.labelsize': 14,
    'legend.fontsize': 14,
    'xtick.major.size': 14,
    'ytick.major.size': 14,
    'xtick.major.width': 1.5,
    'ytick.major.width': 1.5
})

PROTOCOLS = ["MI", "MSI", "MESI", "MOESI", "MESIF"]
COHERENCE_TYPES = ["SNOOP", "DIRECTORY"]
ACCESS_PATTERNS = ["false_sharing", "producer_consumer", "multiple_writers", 
                  "multiple_readers", "random", "no_sharing"]
PROCESSOR_COUNTS = [4, 8, 16, 32, 64, 128]
NUM_ACCESSES = 50

RESULTS_DIR = "evaluation_results"

# This will call the tracegen script to generate traces for each exp
def generate_traces(pattern, num_procs, num_accesses, cache_line_size=64):
    parent_dir = os.path.join(os.path.dirname(os.getcwd()), "build/traces_temp")
    os.makedirs(parent_dir, exist_ok=True)
    
    cmd = [
        "python3", "../tracegen.py",
        "--pattern", pattern,
        "--num-procs", str(num_procs),
        "--num-accesses", str(num_accesses),
        "--output-dir", "traces_temp",
        "--cache-line", str(cache_line_size)
    ]
    
    print(f"Generating traces: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)

def run_mpcsim(protocol, coherence_type, num_procs, directory="traces_temp", diropt=False):
    cmd = [
        "./mpcsim",
        f"--num_procs={num_procs}",
        f"--directory={directory}",
        f"--coherproto={protocol}",
        f"--cohertype={coherence_type}",
        f"--diropt={str(diropt).lower()}",
    ]
    
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        return parse_mpcsim_output(result.stdout)
    except subprocess.CalledProcessError as e:
        print(f"Error running mpcsim: {e}")
        raise

# This func is just to parse the output from the runs of mpcsim
def parse_mpcsim_output(output):
    results = {
        "processors": [],
        "interconnect": {}
    }
    
    proc_pattern = r"PROCESSOR (\d+)\nHITS:\s+(\d+)\nMISSES:\s+(\d+)\nCOHERENCE:\s+(\d+)"
    matches = re.finditer(proc_pattern, output)
    
    for match in matches:
        proc, hits, misses, evictions = match.groups()
        results["processors"].append({
            "id": int(proc),
            "hits": int(hits),
            "misses": int(misses),
            "evictions": int(evictions)
        })
    
    interconnect_pattern = r"INTERCONNECT\nTRAFFIC:\s+(\d+)\nCACHE CONTROL TRAFFIC:\s+(\d+)\nCACHE DATA TRAFFIC:\s+(\d+)\nMEMORY DATA TRAFFIC:\s+(\d+)"
    match = re.search(interconnect_pattern, output)
    
    if match:
        traffic, control_traffic, cache_data_traffic, mem_data_traffic = match.groups()
        results["interconnect"] = {
            "total_traffic": int(traffic),
            "cache_control_traffic": int(control_traffic),
            "cache_data_traffic": int(cache_data_traffic),
            "memory_data_traffic": int(mem_data_traffic)
        }

        # print(match)
    
    config_pattern = r"No of Processors: (\d+)\nDirectory: (.*)\nCoherence Type: (.*)\nCoherence Protocol: (.*)\nCache Line Size: (\d+)\nCache Size: (\d+)"
    match = re.search(config_pattern, output)
    
    if match:
        num_procs, directory, coh_type, coh_proto, cache_line, cache_size = match.groups()
        results["config"] = {
            "num_procs": int(num_procs),
            "directory": directory,
            "coherence_type": coh_type,
            "coherence_protocol": coh_proto,
            "cache_line_size": int(cache_line),
            "cache_size": int(cache_size)
        }
    
    return results

# Run full evaluation with correct separation between SNOOP and DIRECTORY approaches
def run_full_evaluation(output_dir=RESULTS_DIR):
    os.makedirs(output_dir, exist_ok=True)
    results = []
    for pattern in ACCESS_PATTERNS:
        for num_procs in PROCESSOR_COUNTS:
            try:
                generate_traces(pattern, num_procs, NUM_ACCESSES)
            except Exception as e:
                print(f"Failed to generate traces for {pattern} with {num_procs} processors: {e}")
                continue
                
            coherence_type = "SNOOP"
            for protocol in PROTOCOLS:
                try:
                    sim_results = run_mpcsim(protocol, coherence_type, num_procs)
                    sim_results["access_pattern"] = pattern
                    sim_results["diropt"] = False
                    results.append(sim_results)
                    
                    with open(os.path.join(output_dir, "raw_results_incremental.json"), "w") as f:
                        json.dump(results, f, indent=2)
                except Exception as e:
                    config = {
                        "protocol": protocol,
                        "coherence_type": coherence_type,
                        "num_procs": num_procs,
                        "pattern": pattern,
                        "diropt": False
                    }
                    print(f"Error running configuration {config}: {e}")
            
            coherence_type = "DIRECTORY"
            protocol = "MESI"
            
            for diropt in [False, True]:
                try:
                    sim_results = run_mpcsim(protocol, coherence_type, num_procs, diropt=diropt)
                    sim_results["access_pattern"] = pattern
                    sim_results["diropt"] = diropt
                    results.append(sim_results)
                    
                    with open(os.path.join(output_dir, "raw_results_incremental.json"), "w") as f:
                        json.dump(results, f, indent=2)
                except Exception as e:
                    config = {
                        "protocol": protocol,
                        "coherence_type": coherence_type,
                        "num_procs": num_procs,
                        "pattern": pattern,
                        "diropt": diropt
                    }
                    print(f"Error running configuration {config}: {e}")
    
    with open(os.path.join(output_dir, "raw_results.json"), "w") as f:
        json.dump(results, f, indent=2)
    
    print(f"Completed {len(results)} configurations")
    
    return results

# Main calc func
def process_results(results):
    processed_data = []
    
    for result in results:
        config = result.get("config", {})
        access_pattern = result.get("access_pattern", "unknown")
        diropt = result.get("diropt", False)
        
        total_hits = sum(p["hits"] for p in result["processors"])
        total_misses = sum(p["misses"] for p in result["processors"])
        total_evictions = sum(p["evictions"] for p in result["processors"])
        total_memory_ops = total_hits + total_misses
        
        hit_rate = total_hits / total_memory_ops if total_memory_ops > 0 else 0
        miss_rate = total_misses / total_memory_ops if total_memory_ops > 0 else 0
        eviction_rate = total_evictions / total_memory_ops if total_memory_ops > 0 else 0
        
        interconnect = result.get("interconnect", {})
        total_traffic = interconnect.get("total_traffic", 0)
        cache_control_traffic = interconnect.get("cache_control_traffic", 0)
        cache_data_traffic = interconnect.get("cache_data_traffic", 0)
        memory_data_traffic = interconnect.get("memory_data_traffic", 0)
        
        traffic_per_op = total_traffic / total_memory_ops if total_memory_ops > 0 else 0
        data_ratio = cache_data_traffic / total_traffic if total_traffic > 0 else 0
        
        row = {
            "coherence_protocol": config.get("coherence_protocol", ""),
            "coherence_type": config.get("coherence_type", ""),
            "num_procs": int(config.get("num_procs", 0)),
            "access_pattern": access_pattern,
            "diropt": bool(diropt),
            "total_hits": int(total_hits),
            "total_misses": int(total_misses),
            "total_evictions": int(total_evictions),
            "hit_rate": float(hit_rate),
            "miss_rate": float(miss_rate),
            "eviction_rate": float(eviction_rate),
            "total_traffic": int(total_traffic),
            "cache_control_traffic": int(cache_control_traffic),
            "cache_data_traffic": int(cache_data_traffic),
            "memory_data_traffic": int(memory_data_traffic),
            "traffic_per_memory_op": float(traffic_per_op),
            "data_to_total_traffic_ratio": float(data_ratio)
        }
        
        processed_data.append(row)
    
    df = pd.DataFrame(processed_data)
    
    numeric_cols = ["num_procs", "total_hits", "total_misses", "total_evictions", 
                    "hit_rate", "miss_rate", "eviction_rate", "total_traffic", 
                    "cache_control_traffic", "cache_data_traffic", "memory_data_traffic", 
                    "traffic_per_memory_op", "data_to_total_traffic_ratio"]
    
    for col in numeric_cols:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')
    
    return df

# All plotting below
def create_plots(df, output_dir=RESULTS_DIR):
    plots_dir = os.path.join(output_dir, "plots")
    os.makedirs(plots_dir, exist_ok=True)
    
    if df.empty:
        return
    
    def safe_plot(plot_function, filename, title):
        try:
            plt.figure(figsize=(14, 8))
            plot_function()
            plt.title(title, fontsize=18, weight='bold')
            plt.tight_layout()
            plt.savefig(os.path.join(plots_dir, filename))
            plt.close()
            print(f"Created plot: {filename}")
        except Exception as e:
            print(f"Error creating {filename}: {e}")
    
    def plot_hit_rate():
        sns.barplot(x="coherence_protocol", y="hit_rate", hue="access_pattern", data=df)
        plt.xlabel("Coherence Protocol", fontsize=16, weight='bold')
        plt.ylabel("Hit Rate", fontsize=16, weight='bold')
        plt.ylim(0, 1)
        plt.xticks(fontsize=14)
        plt.yticks(fontsize=14)
        plt.legend(title="Access Pattern", bbox_to_anchor=(1.05, 1), loc="upper left", fontsize=14)
    
    safe_plot(plot_hit_rate, "hit_rate_by_protocol_pattern.png", 
             "Cache Hit Rate by Protocol and Access Pattern")
    
    def plot_traffic():
        traffic_df = df.groupby(["coherence_protocol", "access_pattern"], as_index=False)[["total_traffic"]].mean()
        sns.barplot(x="coherence_protocol", y="total_traffic", hue="access_pattern", data=traffic_df)
        plt.xlabel("Coherence Protocol", fontsize=16, weight='bold')
        plt.ylabel("Average Total Traffic", fontsize=16, weight='bold')
        plt.xticks(fontsize=14)
        plt.yticks(fontsize=14)
        plt.legend(title="Access Pattern", bbox_to_anchor=(1.05, 1), loc="upper left", fontsize=14)
    
    safe_plot(plot_traffic, "traffic_by_protocol_pattern.png",
             "Total Interconnect Traffic by Protocol and Access Pattern")
    
    # Switched the x-axis with the pattern from legends
    def plot_evictions():
        eviction_df = df[df["num_procs"] == 8]
        sns.barplot(x="access_pattern", y="total_evictions", hue="coherence_protocol", data=eviction_df)
        plt.xlabel("Access Pattern", fontsize=16, weight='bold')
        plt.ylabel("Coherence Evictions", fontsize=16, weight='bold')
        plt.xticks(rotation=45, fontsize=14)
        plt.yticks(fontsize=14)
        plt.legend(title="Protocol", bbox_to_anchor=(1.05, 1), loc="upper left", fontsize=14)
    
    safe_plot(plot_evictions, "coherence_evictions_by_pattern.png",
             "Coherence-Related Evictions by Access Pattern and Protocol")
    
    for pattern in df["access_pattern"].unique():
        def plot_scalability():
            pattern_df = df[df["access_pattern"] == pattern]
            
            line_styles = ['-', '--', '-.', ':', '-']
            markers = ['o', 's', '^', 'D', 'X', '*']
            colors = ['#3cb44b', '#ffe119', '#4363d8', '#f58231', '#911eb4', '#46f0f0', '#e6194b']
            
            plt.figure(figsize=(20, 14))
            
            groups = pattern_df.groupby(['coherence_protocol', 'coherence_type'])
            
            for i, ((protocol, coh_type), group) in enumerate(groups):
                line_style = line_styles[i % len(line_styles)]
                marker = markers[i % len(markers)]
                color = colors[i % len(colors)]
                
                if coh_type == 'DIRECTORY':
                    for diropt, subgroup in group.groupby('diropt'):
                        label = f"{protocol}-{coh_type}" + (f"-OPT" if diropt else "")
                        plt.plot(subgroup["num_procs"], subgroup["total_traffic"] / 1e7, 
                                 linestyle=line_style, marker=marker, color=color,
                                 linewidth=3, markersize=10, label=label, 
                                 alpha=0.8 if diropt else 1.0)
                else:
                    label = f"{protocol}-{coh_type}"
                    plt.plot(group["num_procs"], group["total_traffic"] / 1e7, 
                             linestyle=line_style, marker=marker, color=color,
                             linewidth=3, markersize=10, label=label)
            
            plt.grid(True, linestyle='--', alpha=0.7)
            
            plt.legend(title="Protocol-Coherence Type", bbox_to_anchor=(1.05, 1), 
                      loc="upper left", frameon=True, framealpha=0.95, fontsize=16)
            
            plt.xlabel("Number of Processors", fontsize=18)
            plt.ylabel("Total Traffic (millions)", fontsize=18)
            
            plt.xticks(fontsize=16)
            plt.yticks(fontsize=16)
            
            plt.gca().xaxis.label.set_weight('bold')
            plt.gca().yaxis.label.set_weight('bold')
            plt.title(f"Scalability: Traffic vs. Number of Processors ({pattern})", fontsize=20, weight='bold')
            
            plt.annotate(f"Access Pattern: {pattern}", xy=(0.02, 0.95), 
                        xycoords='axes fraction', fontsize=16, 
                        bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="gray", alpha=0.8))
        
        safe_plot(plot_scalability, f"scalability_{pattern}.png", 
                 f"Scalability: Traffic vs. Number of Processors ({pattern})")
        
        def plot_directory_scalability_for_producer_consumer():
            pc_dir_df = df[(df["access_pattern"] == "producer_consumer") & 
                        (df["coherence_type"] == "DIRECTORY")]
            
            if pc_dir_df.empty:
                print("No data foun")
                return
            
            line_styles = ['-', '--']
            markers = ['o', 's']
            colors = ['#4363d8', '#e6194b']
            
            plt.figure(figsize=(20, 14))
            
            for j, (diropt, subgroup) in enumerate(pc_dir_df.groupby('diropt')):
                line_style = line_styles[j % len(line_styles)]
                marker = markers[j % len(markers)]
                color = colors[j % len(colors)]
                
                label = f"DIRECTORY" + (f"-OPT" if diropt else "")
                plt.plot(subgroup["num_procs"], subgroup["total_traffic"] / 1e7, 
                        linestyle=line_style, marker=marker, color=color,
                        linewidth=3, markersize=10, label=label)
            
            plt.grid(True, linestyle='--', alpha=0.7)
            
            plt.legend(title="Directory Type", bbox_to_anchor=(1.05, 1), 
                    loc="upper left", frameon=True, framealpha=0.95, fontsize=16)

            plt.xlabel("Number of Processors", fontsize=18)
            plt.xticks(fontsize=16)
            plt.yticks(fontsize=16)
            plt.ylabel("Total Traffic (millions)", fontsize=18)
            plt.title("Directory vs. Directory-OPT Scalability: Producer-Consumer Pattern", fontsize=20)
            
            plt.gca().xaxis.label.set_weight('bold')
            plt.gca().yaxis.label.set_weight('bold')
            plt.gca().title.set_weight('bold')
            
            plt.annotate("Access Pattern: producer_consumer", xy=(0.02, 0.95), 
                        xycoords='axes fraction', fontsize=16, 
                        bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="gray", alpha=0.8))

        safe_plot(plot_directory_scalability_for_producer_consumer, 
                "directory_scalability_producer_consumer.png",
                "Directory vs. Directory-OPT Scalability: Producer-Consumer Pattern")
    
    for protocol in df["coherence_protocol"].unique():
        def plot_traffic_breakdown():
            protocol_df = df[df["coherence_protocol"] == protocol]
            traffic_cols = ["cache_control_traffic", "cache_data_traffic", "memory_data_traffic"]
            
            breakdown_df = protocol_df.groupby(["num_procs", "coherence_type"], as_index=False)[traffic_cols].mean()
            breakdown_df = pd.melt(breakdown_df, id_vars=["num_procs", "coherence_type"], 
                                 value_vars=traffic_cols, var_name="Traffic Type", value_name="Traffic")
            
            sns.barplot(x="num_procs", y="Traffic", hue="Traffic Type", data=breakdown_df)
            plt.xlabel("Number of Processors", fontsize=16, weight='bold')
            plt.ylabel("Traffic", fontsize=16, weight='bold')
            plt.xticks(fontsize=14)
            plt.yticks(fontsize=14)
            plt.legend(title="Traffic Type", bbox_to_anchor=(1.05, 1), loc="upper left", fontsize=14)
        
        safe_plot(plot_traffic_breakdown, f"traffic_breakdown_{protocol}.png",
                 f"Traffic Breakdown for {protocol}")
    
    def plot_coherence_comparison():
        sns.barplot(x="coherence_protocol", y="total_traffic", hue="coherence_type", data=df)
        plt.xlabel("Coherence Protocol", fontsize=16, weight='bold')
        plt.ylabel("Average Total Traffic", fontsize=16, weight='bold')
        plt.xticks(fontsize=14)
        plt.yticks(fontsize=14)
        plt.legend(title="Coherence Type", fontsize=14)
    
    safe_plot(plot_coherence_comparison, "snooping_vs_directory.png",
             "Snooping vs. Directory: Total Traffic by Protocol")
    
    # Fixed this to false sharing
    # Feel free to change size of fig because
    # it may show up as too wide.
    dir_df = df[df["coherence_type"] == "DIRECTORY"]
    if not dir_df.empty:
        def plot_dir_optimization():
            pattern = "false_sharing"
            pattern_dir_df = dir_df[dir_df["access_pattern"] == pattern]
            
            procs_df = pattern_dir_df[pattern_dir_df["num_procs"] == 8]
            
            sns.barplot(x="diropt", y="total_traffic", data=procs_df)
            plt.xlabel("Directory Optimization", fontsize=16, weight='bold')
            plt.ylabel("Total Traffic", fontsize=16, weight='bold')
            plt.xticks([0, 1], ["Off", "On"], fontsize=14)
            plt.yticks(fontsize=14)
            plt.title(f"Impact of Directory Optimization on Traffic - {pattern} (8 Processors)", fontsize=18, weight='bold')
        
        safe_plot(plot_dir_optimization, "directory_optimization_impact.png",
                 "Impact of Directory Optimization on Traffic (False Sharing Pattern)")
    
    compare_df = df[(df["coherence_protocol"].isin(["MESI", "MSI"])) & 
                   (df["coherence_type"] == "SNOOP")]
    if not compare_df.empty:
        def plot_protocol_comparison():
            sns.barplot(x="access_pattern", y="hit_rate", hue="coherence_protocol", data=compare_df)
            plt.xlabel("Access Pattern", fontsize=16, weight='bold')
            plt.ylabel("Hit Rate", fontsize=16, weight='bold')
            plt.ylim(0, 1)
            plt.xticks(rotation=45, fontsize=14)
            plt.yticks(fontsize=14)
            plt.legend(title="Protocol", fontsize=14)
        
        safe_plot(plot_protocol_comparison, "mesi_vs_msi_hitrate.png",
                 "MESI vs. MSI: Hit Rate Comparison")
    
    def plot_traffic_efficiency():
        sns.boxplot(x="coherence_protocol", y="traffic_per_memory_op", data=df)
        plt.xlabel("Coherence Protocol", fontsize=16, weight='bold')
        plt.ylabel("Traffic per Memory Operation", fontsize=16, weight='bold')
        plt.xticks(fontsize=14)
        plt.yticks(fontsize=14)
    
    safe_plot(plot_traffic_efficiency, "traffic_efficiency.png",
             "Traffic Efficiency: Traffic per Memory Operation")
    
    # plotted it this way because it seemed cleaner than having multi variable bars
    def plot_hit_miss_eviction_rates():
        fixed_procs_df = df[df["num_procs"] == 8]
        
        for pattern in fixed_procs_df["access_pattern"].unique():
            pattern_df = fixed_procs_df[fixed_procs_df["access_pattern"] == pattern]
            
            metrics_df = pattern_df.copy()
            metrics_df = metrics_df[metrics_df["coherence_type"] == "SNOOP"]
            
            melted_df = pd.melt(
                metrics_df, 
                id_vars=["coherence_protocol"],
                value_vars=["hit_rate", "miss_rate", "eviction_rate"],
                var_name="Metric", 
                value_name="Rate"
            )
            
            plt.figure(figsize=(16, 10))
            g = sns.barplot(
                x="coherence_protocol", 
                y="Rate", 
                hue="Metric", 
                data=melted_df,
                palette=["#2ecc71", "#e74c3c", "#3498db"] # Feel free to change these if visibiluty is bad
            )
            
            plt.title(f"Cache Performance Metrics for {pattern} (8 Processors)", fontsize=18, weight='bold')
            plt.xlabel("Coherence Protocol", fontsize=16, weight='bold')
            plt.ylabel("Rate", fontsize=16, weight='bold')
            plt.ylim(0, 1)
            
            plt.xticks(fontsize=14)
            plt.yticks(fontsize=14)
            
            for container in g.containers:
                g.bar_label(container, fmt='%.2f', fontsize=12)
            
            plt.legend(title="Metric", labels=["Hit Rate", "Miss Rate", "Eviction Rate"], fontsize=14)
            plt.tight_layout()
            plt.savefig(os.path.join(plots_dir, f"hit_miss_eviction_rates_{pattern}.png"))
            plt.close()
            print(f"Created plot: hit_miss_eviction_rates_{pattern}.png")
    
    try:
        plot_hit_miss_eviction_rates()
    except Exception as e:
        print(f"Error creating hit/miss/eviction rate plots: {e}")
    
    # def plot_protocol_metrics_by_pattern():
        
    #     filtered_df = df[(df["num_procs"] == 8) & (df["coherence_type"] == "SNOOP")]
        
    #     access_patterns = filtered_df["access_pattern"].unique()
    #     protocols = filtered_df["coherence_protocol"].unique()
        
    #     plt.figure(figsize=(20, 12))
        
    #     colors = {
    #         "hit_rate": "#2ecc71",
    #         "miss_rate": "#e74c3c",
    #         "eviction_rate": "#3498db"
    #     }
        
    #     pattern_width = 0.8
    #     protocol_width = pattern_width / len(protocols)
    #     bar_width = protocol_width / 3
        
    #     x = np.arange(len(access_patterns))
        
    #     legend_handles = []
        
    #     for p_idx, protocol in enumerate(protocols):
    #         protocol_df = filtered_df[filtered_df["coherence_protocol"] == protocol]
            
    #         hit_rates = []
    #         miss_rates = []
    #         eviction_rates = []
            
    #         for pattern in access_patterns:
    #             pattern_df = protocol_df[protocol_df["access_pattern"] == pattern]
    #             if not pattern_df.empty:
    #                 hit_rates.append(pattern_df["hit_rate"].values[0])
    #                 miss_rates.append(pattern_df["miss_rate"].values[0])
    #                 eviction_rates.append(pattern_df["eviction_rate"].values[0])
    #             else:
    #                 hit_rates.append(0)
    #                 miss_rates.append(0)
    #                 eviction_rates.append(0)
            
    #         protocol_offset = (p_idx - len(protocols)/2 + 0.5) * protocol_width
            
    #         hit_bars = plt.bar(
    #             x + protocol_offset - bar_width, 
    #             hit_rates, 
    #             bar_width, 
    #             color=colors["hit_rate"], 
    #             label=f"{protocol} - Hit Rate" if p_idx == 0 else ""
    #         )
            
    #         miss_bars = plt.bar(
    #             x + protocol_offset, 
    #             miss_rates, 
    #             bar_width, 
    #             color=colors["miss_rate"], 
    #             label=f"{protocol} - Miss Rate" if p_idx == 0 else ""
    #         )
            
    #         eviction_bars = plt.bar(
    #             x + protocol_offset + bar_width, 
    #             eviction_rates, 
    #             bar_width, 
    #             color=colors["eviction_rate"], 
    #             label=f"{protocol} - Eviction Rate" if p_idx == 0 else ""
    #         )
            
    #         if p_idx == 0:
    #             legend_handles.extend([hit_bars, miss_bars, eviction_bars])
            
    #         for i, v in enumerate(hit_rates):
    #             plt.text(x[i] + protocol_offset - bar_width, v + 0.02, f"{v:.2f}", ha='center', fontsize=8)
    #         for i, v in enumerate(miss_rates):
    #             plt.text(x[i] + protocol_offset, v + 0.02, f"{v:.2f}", ha='center', fontsize=8)
    #         for i, v in enumerate(eviction_rates):
    #             plt.text(x[i] + protocol_offset + bar_width, v + 0.02, f"{v:.2f}", ha='center', fontsize=8)
        
    #     protocol_handles = [plt.Rectangle((0,0),1,1, color=f"C{i}") for i in range(len(protocols))]
    #     protocol_legend = plt.legend(protocol_handles, protocols, 
    #                                title="Protocols", loc="upper right",
    #                                bbox_to_anchor=(1.15, 1))
    #     plt.gca().add_artist(protocol_legend)
        
    #     metric_handles = [plt.Rectangle((0,0),1,1, color=color) for color in colors.values()]
    #     plt.legend(metric_handles, ["Hit Rate", "Miss Rate", "Eviction Rate"], 
    #               title="Metrics", loc="upper right",
    #               bbox_to_anchor=(1.15, 0.8))
        
    #     plt.xlabel('Access Pattern', fontsize=14)
    #     plt.ylabel('Rate', fontsize=14)
    #     plt.title('Cache Performance Metrics by Protocol and Access Pattern (8 Processors)', fontsize=16)
        
    #     plt.xticks(x, access_patterns, rotation=45, ha='right')
    #     plt.ylim(0, 1.1)
        
    #     plt.grid(axis='y', linestyle='--', alpha=0.7)
    
    # safe_plot(plot_protocol_metrics_by_pattern, "protocol_metrics_by_pattern.png",
    #          "Protocol Performance Metrics by Access Pattern")

def analyze_protocol_metrics(df, output_dir=RESULTS_DIR):
    reports_dir = os.path.join(output_dir, "protocol_analysis")
    os.makedirs(reports_dir, exist_ok=True)
    
    analysis = {}
    
    for protocol in df["coherence_protocol"].unique():
        protocol_df = df[df["coherence_protocol"] == protocol]
        
        if protocol_df.empty:
            continue
            
        analysis[protocol] = {
            "avg_hit_rate": float(protocol_df["hit_rate"].mean()),
            "avg_traffic": float(protocol_df["total_traffic"].mean()),
            "avg_evictions": float(protocol_df["total_evictions"].mean()),
            "data_to_control_ratio": float(
                protocol_df["cache_data_traffic"].sum() / 
                protocol_df["cache_control_traffic"].sum() if protocol_df["cache_control_traffic"].sum() > 0 else 0
            )
        }
    
    with open(os.path.join(reports_dir, "protocol_analysis.json"), "w") as f:
        json.dump(analysis, f, indent=2)
    
    metrics = ["avg_hit_rate", "avg_traffic", "avg_evictions", "data_to_control_ratio"]
    metric_labels = {
        "avg_hit_rate": "Average Hit Rate",
        "avg_traffic": "Average Traffic",
        "avg_evictions": "Average Evictions",
        "data_to_control_ratio": "Data to Control Traffic Ratio"
    }
    
    for metric in metrics:
        try:
            values = {protocol: data.get(metric, 0) for protocol, data in analysis.items()}
            
            plt.figure(figsize=(14, 8))
            plt.bar(values.keys(), values.values())
            plt.title(f"Protocol Comparison: {metric_labels.get(metric, metric)}", fontsize=18, weight='bold')
            plt.ylabel(metric_labels.get(metric, metric), fontsize=16, weight='bold')
            plt.xlabel("Protocol", fontsize=16, weight='bold')
            plt.xticks(fontsize=14)
            plt.yticks(fontsize=14)
            plt.grid(axis='y', linestyle='--', alpha=0.7)
            plt.tight_layout()
            plt.savefig(os.path.join(reports_dir, f"{metric}_comparison.png"))
            plt.close()
        except Exception as e:
            print(f"Error {e}")
    
    return analysis

def main():
    global PROCESSOR_COUNTS, ACCESS_PATTERNS, PROTOCOLS, COHERENCE_TYPES
    
    parser = argparse.ArgumentParser(description="Cache Coherence Protocol Evaluation")
    parser.add_argument("--skip-run", action="store_true", help="Skip running simulations and use existing results")
    parser.add_argument("--output-dir", default=RESULTS_DIR, help="Directory to store results")
    parser.add_argument("--max-procs", type=int, default=8, help="Maximum number of processors to test")
    parser.add_argument("--patterns", nargs='+', choices=ACCESS_PATTERNS, 
                      help="Specific access patterns to test (default: all)")
    parser.add_argument("--protocols", nargs='+', choices=PROTOCOLS, 
                      help="Specific protocols to test (default: all)")
    parser.add_argument("--coherence-types", nargs='+', choices=COHERENCE_TYPES, 
                      help="Specific coherence types to test (default: all)")
    args = parser.parse_args()
    
    output_dir = args.output_dir
    
    if args.max_procs:
        PROCESSOR_COUNTS = [p for p in PROCESSOR_COUNTS if p <= args.max_procs]
        print(f"Using processor counts: {PROCESSOR_COUNTS}")
    
    patterns_to_test = args.patterns if args.patterns else ACCESS_PATTERNS
    protocols_to_test = args.protocols if args.protocols else PROTOCOLS
    coherence_types_to_test = args.coherence_types if args.coherence_types else COHERENCE_TYPES
    
    if args.skip_run:
        try:
            try:
                with open(os.path.join(output_dir, "raw_results_incremental.json"), "r") as f:
                    results = json.load(f)
            except FileNotFoundError:
                with open(os.path.join(output_dir, "raw_results.json"), "r") as f:
                    results = json.load(f)
        except FileNotFoundError:
            return
    else:
        ACCESS_PATTERNS = patterns_to_test
        PROTOCOLS = protocols_to_test
        COHERENCE_TYPES = coherence_types_to_test
        
        print(f"Running evaluation with:")
        print(f"  Access patterns: {ACCESS_PATTERNS}")
        print(f"  Protocols (for snooping): {PROTOCOLS}")
        print(f"  Coherence types: {COHERENCE_TYPES}")
        print(f"  Processor counts: {PROCESSOR_COUNTS}")
        
        results = run_full_evaluation(output_dir)
    
    if not results:
        return
    
    try:
        df = process_results(results)
        
        df.to_csv(os.path.join(output_dir, "processed_results.csv"), index=False)
        create_plots(df, output_dir)
        analyze_protocol_metrics(df, output_dir)
    except Exception as e:
        print(f"Error: {e}")
        return

if __name__ == "__main__":
    main()