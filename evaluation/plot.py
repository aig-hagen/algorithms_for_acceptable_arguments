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

if __name__ == "__main__":
    # Check if file path is provided as command-line argument
    if len(sys.argv) != 4:
        print("Usage: python script.py <file_path> <output_file>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    benchmark = sys.argv[2] #"ICCMA19"
    problem = sys.argv[3] #"EC-ST"
    timeout = 1200
    output_file = benchmark.lower() + "_" + problem.lower() + ".pgf"
    
    # Prepare DataFrame
    dataframe = read_csv_to_dataframe(file_path)
    dataframe = dataframe[dataframe["task"] == problem]
    dataframe = dataframe[dataframe["benchmark_name"] == benchmark]

    dataframe = dataframe[dataframe["solver_name"] != "FUDGE-cadical"]

    # Compute Virtual Best Solver
    for instance in dataframe["instance"].unique():
        view = dataframe[dataframe["instance"] == instance]
        view = view[["solver_name", "runtime"]].set_index("solver_name")
        rt=view["runtime"].min()
        contrib = view.index[view["runtime"] == rt].tolist()
        if len(contrib) == 1:
            contributor = contrib[0]
        else: 
            contributor = None
        row = {"solver_name": "VBS", "instance": instance, "runtime": rt, "task": problem, "benchmark_name": benchmark, "contributor": contributor}
        dataframe = dataframe._append(row, ignore_index=True)

    #print(dataframe)

    # Group by solver
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

    ax.axhline(y=timeout, color='r', linestyle='--', label=None)

    # Add labels and title
    ax.set_xlabel("Instance")
    ax.set_ylabel("Runtime in s")
    #ax.set_title("Line Plot of Runtime for Each Group")
    ax.legend()

    #ax.set_yscale('log')
    
    # Save plot to file
    if output_file.endswith(".pgf"):
        pass#plt.savefig(output_file, format='pgf')
    else:
        raise NameError("Unsupported Output type")
    
    # Create a table with one row for each group
    table_data = []
    for name, group in grouped_dataframe:
        num_rows = len(group)
        timeouts = group["runtime"].eq(timeout).sum()
        total_runtime = group["runtime"].sum() - (timeouts* timeout)
        par_10 = (group['runtime'].sum() + (9 * timeouts * timeout)) / num_rows

        if name != "VBS":
            view = dataframe[dataframe["solver_name"] == "VBS"]
            vbs = view["contributor"].value_counts().get(name, 0)
            name = "\\" + name.split("-")[0]
            if problem.count("EC") > 0:
                name += "$^c$"
            else:
                name += "$^s$"
        else:
            vbs = "-"

        
        table_data.append([name, num_rows, timeouts, total_runtime, par_10, vbs])
    table_df = pd.DataFrame(table_data, columns=["Algorithm", "N", "#TO", "RT", "PAR10", "#VBS"])
    table_df.sort_values(["#TO", "RT"], inplace=True)
    table_df.index = np.arange(1, len(table_df)+1)
    table_df.index.name = "No."
    table_df.reset_index(inplace=True)
    
    # Save table to file
    table_df.to_latex(output_file.replace('.pgf', '_table.tex').replace('.png', '_table.tex'), index=False, float_format="%.2f")