import sys
import os
import re

def compare_lists(list1, list2):
    return set(list1) == set(list2)

def parse_file(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
        # Using regular expression to extract list contents
        match = re.search(r'\[([^[\]]*)\]', content)
        if match:
            list_content = match.group(1)
            parsed_list = [item.strip() for item in list_content.split(',')]
            return parsed_list
        else:
            print(f"Error parsing file {file_path}: No list found in file.")
            return None

def compare_directories(dir1, dir2):
    files1 = os.listdir(dir1)
    files2 = os.listdir(dir2)

    common_files = set(files1) & set(files2)

    incorrect_files = []
    total_files = len(common_files)
    correct_files = 0

    for filename in common_files:
        file1_path = os.path.join(dir1, filename)
        file2_path = os.path.join(dir2, filename)

        if os.path.isfile(file1_path) and os.path.isfile(file2_path):
            list1 = parse_file(file1_path)
            list2 = parse_file(file2_path)

            if list1 is not None and list2 is not None:
                if compare_lists(list1, list2):
                    correct_files += 1
                else:
                    incorrect_files.append(filename)
    
    print(f"Validation Result: {correct_files}/{total_files}")
    print("Incorrect Instances:")
    for filename in incorrect_files:
        print(filename)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python validate.py directory1 directory2")
        sys.exit(1)

    directory1 = sys.argv[1]
    directory2 = sys.argv[2]

    if not os.path.isdir(directory1) or not os.path.isdir(directory2):
        print("Error: Both arguments should be directories.")
        sys.exit(1)

    compare_directories(directory1, directory2)
