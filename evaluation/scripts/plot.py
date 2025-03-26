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
    if len(sys.argv) != 3:
        print("Usage: python script.py <file_path> <output_file>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    problem = sys.argv[2] #"EC-ST"
    timeout = 1200
    output_file = problem.lower() + ".pgf"
    
    # Prepare DataFrame
    dataframe = read_csv_to_dataframe(file_path)
    dataframe = dataframe[dataframe["task"] == problem]

    print(dataframe)

    dataframe = dataframe[dataframe["solver_name"] != "FUDGE-cadical"]

    print(dataframe)

    dataframe = dataframe.astype({'runtime': 'float'})

    print(dataframe)

    dataframe = dataframe[dataframe["timed_out"] == "False"]

    print(dataframe)

    #dataframe = dataframe[dataframe["benchmark_name"] != "ICCMA15"]
    #dataframe = dataframe[dataframe["benchmark_name"] != "ICCMA19"]
    #dataframe = dataframe[dataframe["benchmark_name"] != "ICCMA17"]
    dataframe = dataframe[dataframe["benchmark_name"] == "ICCMA23"]

    print(dataframe)

    # Define markers for each group
    if problem.count("EC") > 0:
        iaq = "IAQ$^c$"
        eee = "EEE$^c$"
        see = "SEE$^c$"
        seem = "SEEM$^c$"
    else:
        iaq = "IAQ$^s$"
        eee = "EEE$^s$"
        see = "SEE$^s$"
        seem = "SEEM$^s$"
    
    style_map = {
        iaq: {'marker': 'x', 'linestyle': '--'},   # Circle, Solid line
        eee: {'marker': 'o', 'linestyle': '-'},  # X, Dashed line
        see: {'marker': '^', 'linestyle': ':'},  # Triangle, Dash-dot line
        seem: {'marker': 'D', 'linestyle': '-.'}    # Diamond, Dotted line
    }

    #colors = {iaq: 'orange', eee: 'blue', see: 'green', seem: 'red'}  # Define colors

    custom_order = ["IAQ-cgss2", "EEE-cgss2", "SEE-cgss2", "SEEM-cgss2"]
    #custom_order = ["IAQ-cgss2", "SEE-cgss2"]
    #custom_order = ["IAQ-cadical","EEE-cadical"]

    # Group by solver
    grouped_dataframe = dataframe.groupby("solver_name")


    # Set up Matplotlib for producing .tex output
    plt.rcParams.update({
        "pgf.texsystem": "pdflatex",
        "pgf.preamble": r"\usepackage{amsmath}"
    })

    # Create figure and axis objects
    fig, ax = plt.subplots()

    for slv in custom_order:
        name = slv
        group = grouped_dataframe.get_group(slv)
    # Print the grouped DataFrame
    #for i, (name, group) in enumerate(grouped_dataframe):
        if name != "VBS":
            name = name.split("-")[0]
            if problem.count("EC") > 0:
                name += "$^c$"
            else:
                name += "$^s$"
        #print(name)
        print(len(group["instance"].tolist()))
        #print(group[group["runtime"] < 5]["benchmark_name"].value_counts())
        #group = group[group["instance"].isin(problem_instances)]
        group = group.sort_values(by="runtime")
        group = group.reset_index(drop=True)
        ax.plot(group["runtime"], group.index, marker=style_map[name]['marker'], markersize=2, markerfacecolor='none', linewidth=1, linestyle=style_map[name]['linestyle'], label=name)

    ax.axvline(x=timeout, color='r', linestyle='--', label=None)

    # Add labels and title
    ax.set_ylabel("instances solved")
    ax.set_xlabel("runtime (s)")
    #ax.set_title("Line Plot of Runtime for Each Group")
    ax.legend(loc='lower right', fontsize=15, markerscale=2.5, handlelength=2, handletextpad=1)

    ax.set_ylim(0, None)
    #ax.set_xscale('log')
    
    # Save plot to file
    if output_file.endswith(".pgf"):
        plt.savefig(output_file, format='pgf')
    elif output_file.endswith(".png"):
        plt.savefig(output_file, format="png")
    else:
        raise NameError("Unsupported Output type")
    

"""
    df_results = pd.DataFrame(columns=["Dataset", "#Instances", "AVG ARGS", "MED ARGS", "STD ARGS", "MIN ARGS", "MAX ARGS", "AVG ATTS", "MED ATTS", "STD ATTS", "MIN ATTS", "MAX ATTS", "AVG DEG"])

    df = pd.read_csv("statistics/ICCMA21.csv")

    row = {"Dataset": "ICCMA21", "#Instances": len(df["Instance"].unique()),
        "AVG ARGS": df["#Args"].mean(),"MED ARGS": df["#Args"].median(), 
        "STD ARGS": df["#Args"].std(),
        "MIN ARGS": df["#Args"].min(),"MAX ARGS": df["#Args"].max(), 
        "AVG ATTS": df["#Attacks"].mean(), "MED ATTS": df["#Attacks"].median(),
        "STD ATTS": df["#Attacks"].std(),
        "MIN ATTS": df["#Attacks"].min(), "MAX ATTS": df["#Attacks"].max(),
        "AVG DEG": df["AVG DEG"].mean(),
        }
    df_results = df_results._append(row, ignore_index=True)

    print(problem_instances)
    df = df[df["Instance"].isin(problem_instances)]

    row = {"Dataset": "ICCMA21", "#Instances": len(df["Instance"].unique()),
        "AVG ARGS": df["#Args"].mean(),"MED ARGS": df["#Args"].median(), 
        "STD ARGS": df["#Args"].std(),
        "MIN ARGS": df["#Args"].min(),"MAX ARGS": df["#Args"].max(), 
        "AVG ATTS": df["#Attacks"].mean(), "MED ATTS": df["#Attacks"].median(),
        "STD ATTS": df["#Attacks"].std(),
        "MIN ATTS": df["#Attacks"].min(), "MAX ATTS": df["#Attacks"].max(),
        "AVG DEG": df["AVG DEG"].mean(),
        }
    df_results = df_results._append(row, ignore_index=True)


    print(df_results)

    df_results.to_latex('table.tex', index=False, float_format="%.0f")"""