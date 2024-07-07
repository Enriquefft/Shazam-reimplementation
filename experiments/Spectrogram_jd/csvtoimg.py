import csv
import numpy as np
from PIL import Image

import csv
import numpy as np
from PIL import Image

def save_image_from_csv(csv_file, output_path):
    try:
        with open(csv_file, 'r') as file:
            reader = csv.reader(file)
            data = list(reader)
            
        # Check if the CSV data has at least the dimensions and some pixel data
        if len(data) < 2:
            raise ValueError("CSV file does not contain enough rows (missing dimensions or pixel data).")

        # Extract image dimensions
        height = int(data[0][0])
        width = int(data[0][1])

        # Check if the dimensions are valid
        if width <= 0 or height <= 0:
            raise ValueError("Invalid dimensions found in the CSV file.")

        # Check if the number of pixel rows matches the specified height
        if len(data) - 1 != height:
            raise ValueError(f"Number of pixel rows ({len(data) - 1}) does not match the specified height ({height}).")

        for row in data[1:]:
            if len(row) != width:
                raise ValueError(f"Row with {len(row)} columns found, expected {width} columns.")

        # Convert pixel data to numpy array
        pixel_data = np.array([[float(val) for val in row] for row in data[1:]])

        # Reshape pixel data to original image dimensions
        pixel_data = (pixel_data * 255).astype(np.uint8)  # Scale to 0-255 range

        # Create image from pixel data
        image = Image.fromarray(pixel_data.reshape(height, width), mode='L')

        # Save image to specified location
        image.save(output_path)

        print(f"Image saved successfully to {output_path}")

    except FileNotFoundError:
        print(f"Error: File '{csv_file}' not found.")
    except ValueError as ve:
        print(f"Error: {ve}")
    except Exception as e:
        print(f"Error: An unexpected error occurred - {str(e)}")
# Example usage:
csv_file = 'Inputs/1mbmono_spec.csv'  # Replace with your CSV file path
output_path = 'Outputs/1mbmono_spec.png'  # Replace with desired output file path
save_image_from_csv(csv_file, output_path)
