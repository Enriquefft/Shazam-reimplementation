import csv
import hashlib

def read_csv(file):
    with open(file, 'r') as f:
        reader = csv.reader(f)
        return {int(row[0]) for row in reader}

def calculate_hashes(values):
    return {hashlib.md5(str(value).encode()).hexdigest() for value in values}

def find_common_values(file1, file2, output_file):
    # Read the CSV files
    values1 = read_csv(file1)
    values2 = read_csv(file2)
    
    # Find common values
    common_values = values1.intersection(values2)
    
    # Save common values to a new CSV file
    with open(output_file, 'w', newline='') as f:
        writer = csv.writer(f)
        for value in common_values:
            writer.writerow([value])
    
    # Calculate distinct hashes on the aggregate data
    all_values = values1.union(values2)
    distinct_hashes = calculate_hashes(all_values)
    
    print(f"Common values have been saved to {output_file}")
    print(f"Number of distinct hashes: {len(distinct_hashes)}")

# Example usage
file1 = 'hashes_clear.csv'
file2 = 'hashes_noisy.csv'
output_file = 'common_values.csv'
find_common_values(file1, file2, output_file)
