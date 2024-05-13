import os
import subprocess
import argparse

def execute_binary_for_files(directory, binary_path, problem):
    # Check if the directory exists
    if not os.path.exists(directory):
        print(f"Directory '{directory}' does not exist.")
        return

    # Check if the binary exists
    if not os.path.exists(binary_path):
        print(f"Binary '{binary_path}' does not exist.")
        return

    binary_name = os.path.basename(binary_path)
    results_directory = os.path.join("results", binary_name)
    i = 1
    while os.path.exists(results_directory):
        i += 1
        results_directory = os.path.join("results", f"{binary_name}_{i}")

    os.makedirs(results_directory, exist_ok=True)

    # Iterate through each file in the directory
    for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        
        # Check if it's a file
        if os.path.isfile(filepath):
            output_file = os.path.join(results_directory, filename + ".out")
            # Execute the binary with the file as an argument
            with open(output_file, "w") as f:
                subprocess.run([binary_path, "-f", filepath, "-fo", "tgf", "-p", problem], stdout=f, stderr=subprocess.STDOUT)

if __name__ == "__main__":
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Run Solver for every instance in directory")
    parser.add_argument("directory", help="Path to the directory containing instances")
    parser.add_argument("binary", help="Path to the solver binary")
    parser.add_argument("problem", help="Problem to be solved")
    args = parser.parse_args()

    # Call the function with command-line arguments
    execute_binary_for_files(args.directory, args.binary, args.problem)