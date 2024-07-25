import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
from pathlib import Path

# Ensure seaborn styles are applied for better aesthetics
sns.set_theme(style="whitegrid")

runs_dir = Path("Runs")

data_frames = []
algorithm_names = []

# Traverse the directory structure
for algorithm_dir in os.listdir(runs_dir):
    if algorithm_dir.startswith("Z_"):
        continue

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

# Use a color palette for better distinction
palette = sns.color_palette("husl", len(algorithm_names))

for metric in metrics:
    plt.figure(figsize=(14, 8))
    sns.boxplot(x="algorithm", y=metric, data=combined_df, palette=palette)
    plt.title(f"Comparison of {metric} across algorithms")
    plt.xlabel("Algorithm")
    plt.ylabel(metric)
    plt.xticks(rotation=45, ha="right")
    plt.grid(True, linestyle="--", alpha=0.7)
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

# Additional summary plot for mean values of each metric
mean_summary = combined_df.groupby("algorithm")[metrics].mean().reset_index()
mean_summary_melted = mean_summary.melt(
    id_vars="algorithm", var_name="metric", value_name="mean_value"
)

plt.figure(figsize=(14, 8))
sns.barplot(
    x="mean_value",
    y="algorithm",
    hue="metric",
    data=mean_summary_melted,
    palette="viridis",
)
plt.title("Mean values of metrics across algorithms")
plt.xlabel("Mean Value")
plt.ylabel("Algorithm")
plt.legend(loc="upper right")
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.savefig("mean_values_summary.png")
plt.show()
#
# import pandas as pd
# import matplotlib.pyplot as plt
# import os
#
# from pathlib import Path
#
# runs_dir = Path("Runs")
#
# data_frames = []
# algorithm_names = []
#
# # Traverse the directory structure
# for algorithm_dir in os.listdir(runs_dir):
#     if algorithm_dir.startswith("Z_"):
#         continue
#
#     algorithm_path = os.path.join(
#         runs_dir, algorithm_dir, "data", "song_statistics.csv"
#     )
#     if os.path.exists(algorithm_path):
#         algorithm_name = algorithm_dir
#         df = pd.read_csv(algorithm_path)
#         df["algorithm"] = algorithm_name
#         data_frames.append(df)
#         algorithm_names.append(algorithm_name)
#
# # Combine all DataFrames into a single DataFrame
# combined_df = pd.concat(data_frames, ignore_index=True)
#
# # Display the combined DataFrame
# print(combined_df.head())
#
# # Save the combined DataFrame to a CSV file
# combined_df.to_csv("combined_data.csv", index=False)
#
# # Calculate summary statistics for each algorithm
# summary_stats = combined_df.groupby("algorithm").describe()
# print(summary_stats)
#
# # Plot comparisons of different metrics across algorithms
# metrics = [
#     "feature_count",
#     "hash_count",
#     "time_to_read",
#     "stft_time",
#     "local_maxima_time",
#     "hash_time",
#     "dump_time",
# ]
#
# for metric in metrics:
#     plt.figure(figsize=(12, 6))
#     combined_df.boxplot(column=metric, by="algorithm")
#     plt.title(f"Comparison of {metric} across algorithms")
#     plt.suptitle("")
#     plt.xlabel("Algorithm")
#     plt.ylabel(metric)
#     plt.xticks(rotation=45)
#     plt.tight_layout()
#     plt.savefig(f"{metric}_comparison.png")
#     plt.show()
#
# # Create tables for the summary statistics and save as CSV
# summary_stats.to_csv("summary_statistics.csv")
#
# # Display summary statistics for each metric
# for metric in metrics:
#     metric_summary = combined_df.groupby("algorithm")[metric].describe()
#     print(metric_summary)
#     metric_summary.to_csv(f"{metric}_summary.csv")
