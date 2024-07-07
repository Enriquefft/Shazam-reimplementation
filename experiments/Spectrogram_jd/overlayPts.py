import matplotlib.pyplot as plt
import numpy as np

def plot_critical_points(image_path, csv_path, output_path):
    # Load the image
    image = plt.imread(image_path)
    if image is None:
        print("Error loading image")
        return
    
    # Load the CSV file without headers
    critical_points = np.loadtxt(csv_path, delimiter=',')
    
    # Check if the CSV file has at least two columns
    if critical_points.shape[1] < 2:
        print("CSV file must contain at least two columns for 'x' and 'y' coordinates")
        return
    
    # Plot the image
    plt.imshow(image, cmap='gray')
    
    # Plot the critical points as red crosses (swap y and x)
    plt.scatter(critical_points[:, 0], critical_points[:, 1], color='red', marker='x')
    
    # Save the output image with critical points
    plt.show()

# Example usage
image_path = 'Outputs/1mb_spec_librosa.png'
csv_path = 'Inputs/1mb_crits_librosa.csv'
output_path = 'Output/1mb_crits_img.png'
plot_critical_points(image_path, csv_path, output_path)
