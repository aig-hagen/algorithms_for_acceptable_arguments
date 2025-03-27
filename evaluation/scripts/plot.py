import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
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

def filter_dataframe(dataframe: pd.DataFrame, benchmark: str, problem: str):
    df = dataframe[dataframe["task"] == problem]
    df = df[df["benchmark_name"] == benchmark]
    return df

def sanitize_dataframe(dataframe: pd.DataFrame):
    df = dataframe
    return df


if __name__ == "__main__":
    # Check if file path is provided as command-line argument
    if len(sys.argv) != 4:
        print("Usage: python script.py <file_path> <output_file>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    benchmark = sys.argv[2] #"ICCMA17"
    problem = sys.argv[3] #"EC-ST"

    timeout = 1200
    enable_vbs = False
    output_file = benchmark.lower() + "_" + problem.lower() + ".png"

    
    # Prepare DataFrame
    dataframe = read_csv_to_dataframe(file_path)
    dataframe = filter_dataframe(dataframe, benchmark, problem)
    dataframe = dataframe.astype({'runtime': 'float'})

    dataframe = dataframe[dataframe["timed_out"] == "False"]

    style_map = [
        {'marker': 'x', 'linestyle': '--'},
        {'marker': 'o', 'linestyle': '-'},
        {'marker': '^', 'linestyle': ':'},
        {'marker': 'D', 'linestyle': '-.'}
    ]

    # Group by solver
    grouped_dataframe = dataframe.groupby("solver_name")


    # Set up Matplotlib for producing .tex output
    plt.rcParams.update({
        "pgf.texsystem": "pdflatex",
        "pgf.preamble": r"\usepackage{amsmath}"
    })

    # Create figure and axis objects
    fig, ax = plt.subplots()

    for i, (name, group) in enumerate(grouped_dataframe):
        group = group.sort_values(by="runtime")
        group = group.reset_index(drop=True)
        ax.plot(group["runtime"], group.index, marker=style_map[i]['marker'], markersize=2, markerfacecolor='none', linewidth=1, linestyle=style_map[i]['linestyle'], label=name)

    ax.axvline(x=timeout, color='r', linestyle='--', label=None)

    # Add labels and title
    ax.set_ylabel("instances solved")
    ax.set_xlabel("runtime (s)")
    ax.legend(loc='lower right', fontsize=15, markerscale=2.5, handlelength=2, handletextpad=1)
    ax.set_ylim(0, None)
    
    # Save plot to file
    if output_file.endswith(".pgf"):
        plt.savefig(output_file, format='pgf')
    elif output_file.endswith(".png"):
        plt.savefig(output_file, format="png")
    else:
        raise NameError("Unsupported Output type")