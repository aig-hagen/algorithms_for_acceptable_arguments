import pandas as pd
import matplotlib.pyplot as plt
import sys

def read_csv_to_dataframe(file_path):
    try:
        # Read CSV into a pandas DataFrame
        df = pd.read_csv(file_path)
        return df
    except FileNotFoundError:
        print("File not found. Please check the file path and try again.")
        sys.exit(1)
    except Exception as e:
        print("An error occurred:", e)
        sys.exit(1)

if __name__ == "__main__":
    # Check if file path is provided as command-line argument
    if len(sys.argv) != 3:
        print("Usage: python script.py <file_path> <output_file>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    output_file = sys.argv[2]
    
    dataframe = read_csv_to_dataframe(file_path)

    # Group DataFrame by the "name" column
    grouped_dataframe = dataframe.groupby("solver_name")
    
    # Define markers for each group
    markers = ['o', 's', '^', 'x', '*', '+', 'D', 'v', '>', '<']

    # Set up Matplotlib for producing .tex output
    plt.rcParams.update({
        "pgf.texsystem": "pdflatex",
        "pgf.preamble": r"\usepackage{amsmath}"
    })

    # Create figure and axis objects
    fig, ax = plt.subplots()

    # Print the grouped DataFrame
    for i, (name, group) in enumerate(grouped_dataframe):
        group = group.sort_values(by="runtime")
        group = group.reset_index(drop=True)
        ax.plot(group.index, group["runtime"], marker=markers[i % len(markers)], markersize=5, linewidth=1, label=name)

    ax.axhline(y=1200, color='r', linestyle='--', label=None)

    # Add labels and title
    ax.set_xlabel("Instance")
    ax.set_ylabel("Runtime in s")
    #ax.set_title("Line Plot of Runtime for Each Group")
    ax.legend()

    #ax.set_yscale('log')
    
    # Save plot to file
    if output_file.endswith(".pgf"):
        plt.savefig(output_file, format='pgf')
    elif output_file.endswith(".png"):
        plt.savefig(output_file)
    else:
        raise NameError("Unsupported Output type")