import csv
import matplotlib.pyplot as plt

def read_csv(file):
    with open(file, 'r') as f:
        reader = csv.reader(f)
        return {row[0]: int(row[1]) for row in reader}

def find_common_hashes(file1, file2):
    # Read the CSV files
    data1 = read_csv(file1)
    data2 = read_csv(file2)
    
    # Find common hashes
    common_hashes = set(data1.keys()).intersection(set(data2.keys()))
    
    # Extract times for common hashes
    times1 = [data1[hash] for hash in common_hashes]
    times2 = [data2[hash] for hash in common_hashes]
    
    return times1, times2

def plot_common_hashes(times1, times2):
    plt.scatter(times1, times2)
    plt.xlabel('Time1')
    plt.ylabel('Time2')
    plt.title('Common Hashes Times Plot')
    plt.show()

# Example usage
file1 = 'hashes_clear.csv'
file2 = 'hashes_noisy.csv'

times1, times2 = find_common_hashes(file1, file2)
plot_common_hashes(times1, times2)
