import os
import json

from pathlib import Path


def parse_ini_file(file_path):
    params = {}
    with open(file_path, "r") as file:
        for line in file:
            line = line.strip()
            if line and not line.startswith(";"):
                key, value = line.split("=")
                key = key.strip().lower()
                value = value.strip()
                if "." in value:
                    value = float(value)
                else:
                    value = int(value) if value.isdigit() else value
                params[key] = value
    return params


def read_description_file(file_path):
    with open(file_path, "r") as file:
        return file.read().strip()


def generate_json_from_folders(root_folder):
    runs = []
    for folder_name in os.listdir(root_folder):
        folder_path = os.path.join(root_folder, folder_name)
        if os.path.isdir(folder_path):
            ini_file = os.path.join(folder_path, "settings.ini")
            description_file = os.path.join(folder_path, "description")

            if os.path.exists(ini_file) and os.path.exists(description_file):
                hyperparameters = parse_ini_file(ini_file)
                description = read_description_file(description_file)

                run = {
                    "file": folder_name,
                    "description": description,
                    "hyperparameters": hyperparameters,
                }
                runs.append(run)

    result = {"runs": runs}
    return result


def main():
    root_folder = Path(".")  # Change this to the path of your folders
    result_json = generate_json_from_folders(root_folder)

    with open("result.json", "w") as json_file:
        json.dump(result_json, json_file, indent=4)


if __name__ == "__main__":
    main()
