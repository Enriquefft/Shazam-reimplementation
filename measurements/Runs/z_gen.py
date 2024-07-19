import os
import shutil

from pathlib import Path
# Set the source directory where the original folders are located
source_directory = Path('.')

# Set the destination directory where the copied folders will be placed
destination_directory = Path('.')

# Get a list of all folders in the source directory
folders = [f for f in os.listdir(source_directory) if os.path.isdir(os.path.join(source_directory, f))]

# Copy each folder and rename with the prefix 'Z_'
for folder in folders:
    source_folder = os.path.join(source_directory, folder)
    destination_folder = os.path.join(destination_directory, f'Z_{folder}')
    shutil.copytree(source_folder, destination_folder)
    print(f'Copied {source_folder} to {destination_folder}')

print('All folders have been copied and renamed.')
