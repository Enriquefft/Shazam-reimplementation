from PIL import Image
import csv

def image_to_csv(image_path, csv_path):
    # Open the image file
    with Image.open(image_path) as img:
        # Convert the image to grayscale
        grayscale_img = img.convert("L")
        
        # Get image dimensions
        width, height = grayscale_img.size
        
        # Open a CSV file to write the pixel values
        with open(csv_path, mode='w', newline='') as file:
            writer = csv.writer(file)
            
            # Write the dimensions as the first row
            writer.writerow([width, height])
            
            # Write the pixel values row by row
            for y in range(height):
                row = []
                for x in range(width):
                    pixel_value = grayscale_img.getpixel((x, y))
                    row.append(pixel_value)
                writer.writerow(row)

# Example usage
image_path = 'Outputs/1mb_spec_librosa.png'
csv_path = 'simple.csv'
image_to_csv(image_path, csv_path)
