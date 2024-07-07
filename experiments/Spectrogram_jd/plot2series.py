import matplotlib.pyplot as plt

def plot_csv_files(csv_file1, csv_file2):
    # Read data from CSV file 1
    with open(csv_file1, 'r') as file1:
        lines1 = file1.readlines()
        data1 = [line.strip().split(',') for line in lines1]
        data1 = [[float(entry) for entry in row] for row in data1]

    # Read data from CSV file 2
    with open(csv_file2, 'r') as file2:
        lines2 = file2.readlines()
        data2 = [line.strip().split(',') for line in lines2]
        data2 = [[float(entry) for entry in row] for row in data2]

    # Convert data to sets of tuples for comparison
    set1 = set(tuple(point) for point in data1)
    set2 = set(tuple(point) for point in data2)

    # Identify common points
    common_points = set1 & set2

    # Extract x and y values from data
    x1, y1 = zip(*data1)
    x2, y2 = zip(*data2)
    x_common, y_common = zip(*common_points) if common_points else ([], [])

    # Report the amount of points
    print(f"Total points in first dataset: {len(data1)}")
    print(f"Total points in second dataset: {len(data2)}")
    print(f"Total common points: {len(common_points)}")

    # Plotting
    plt.figure(figsize=(10, 6))

    # Plot series from first CSV file
    plt.scatter(x1, y1, label='Clear', color='blue', marker='x')

    # Plot series from second CSV file
    plt.scatter(x2, y2, label='Noisy', marker='o', facecolors='none', edgecolors='red')

    # Plot common points
    plt.scatter(x_common, y_common, label='Common Points', color='green', marker='s')

    # Add labels and title
    plt.xlabel('X-axis')
    plt.ylabel('Y-axis')
    plt.title('Plot of Two Series from CSV Files')
    
    # Add legend
    plt.legend()

    # Display plot
    plt.show()

# Example usage:
csv_file1 = 'crit_vnoisy_MINLISTGTN_hp.csv'
csv_file2 = 'crit_clear_MINLISTGTN_hp.csv'

plot_csv_files(csv_file1, csv_file2)
