import pandas as pd
import matplotlib.pyplot as plt
import os
import glob

from pathlib import Path

runs_dir = Path("Runs")

data_frames = []
algorithm_names = []

# Traverse the directory structure
for algorithm_dir in os.listdir(runs_dir):
    algorithm_path = os.path.join(
        runs_dir, algorithm_dir, "data", "song_statistics.csv"
    )
    if os.path.exists(algorithm_path):
        algorithm_name = algorithm_dir
        df = pd.read_csv(algorithm_path)
        df["algorithm"] = algorithm_name
        data_frames.append(df)
        algorithm_names.append(algorithm_name)

# Combine all DataFrames into a single DataFrame
combined_df = pd.concat(data_frames, ignore_index=True)

# Display the combined DataFrame
print(combined_df.head())

# Save the combined DataFrame to a CSV file
combined_df.to_csv("combined_data.csv", index=False)

# Calculate summary statistics for each algorithm
summary_stats = combined_df.groupby("algorithm").describe()
print(summary_stats)

# Plot comparisons of different metrics across algorithms
metrics = [
    "feature_count",
    "hash_count",
    "time_to_read",
    "stft_time",
    "local_maxima_time",
    "hash_time",
    "dump_time",
]

for metric in metrics:
    plt.figure(figsize=(12, 6))
    combined_df.boxplot(column=metric, by="algorithm")
    plt.title(f"Comparison of {metric} across algorithms")
    plt.suptitle("")
    plt.xlabel("Algorithm")
    plt.ylabel(metric)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(f"{metric}_comparison.png")
    plt.show()

# Create tables for the summary statistics and save as CSV
summary_stats.to_csv("summary_statistics.csv")

# Display summary statistics for each metric
for metric in metrics:
    metric_summary = combined_df.groupby("algorithm")[metric].describe()
    print(metric_summary)
    metric_summary.to_csv(f"{metric}_summary.csv")
