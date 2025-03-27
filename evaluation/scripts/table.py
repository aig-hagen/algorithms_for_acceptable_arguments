import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys

def read_csv_to_dataframe(file_path: str):
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

def restructure_dataframe(dataframe: pd.DataFrame):
    solvers = dataframe["solver_name"].unique().tolist()
    df = pd.DataFrame()
    for slv in solvers:
        view = dataframe[dataframe["solver_name"] == slv]
        df[slv] = view["runtime"].tolist()
    return df

def compute_vbs(dataframe: pd.DataFrame):
    solvers = dataframe.columns.tolist()
    dataframe['contributor'] = dataframe.idxmin(axis=1)
    dataframe['VBS'] = dataframe[solvers].min(axis=1)
    return dataframe

if __name__ == "__main__":
    # Check if file path is provided as command-line argument
    if len(sys.argv) != 4:
        print("Usage: python script.py <file_path> <output_file>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    benchmark = sys.argv[2] #"ICCMA19"
    problem = sys.argv[3] #"EC-ST"
    
    timeout = 1200
    enable_vbs = False
    output_file = benchmark.lower() + "_" + problem.lower() + ".tex"
    
    # Prepare DataFrame
    dataframe = read_csv_to_dataframe(file_path)
    dataframe = filter_dataframe(dataframe, benchmark, problem)
    dataframe = dataframe.astype({'runtime': 'float'})

    #dataframe = dataframe[dataframe["solver_name"] != "FUDGE-cadical"]

    dataframe = restructure_dataframe(dataframe)

    if enable_vbs:
        dataframe = compute_vbs(dataframe)

    table_data = []

    solvers = dataframe.columns.tolist()
    if enable_vbs:
        solvers.remove("contributor")

    for name in solvers:
        num_rows = len(dataframe[name])
        timeouts = dataframe[name].eq(timeout).sum()
        total_runtime = dataframe[name].sum() - (timeouts* timeout)
        par_10 = (dataframe[name].sum() + (9 * timeouts * timeout)) / num_rows
        if enable_vbs:
            if name != "VBS":
                vbs = dataframe["contributor"].value_counts().get(name, 0)
            else:
                vbs = "-"
        #if name != "VBS":
        #    vbs="-"#vbs = new_df["contributor"].value_counts().get(name, 0)
        #    name = "\\" + name.split("-")[0]
        #    if problem.count("EC") > 0:
        #        name += "$^c$"
        #    else:
        #        name += "$^s$"
        #else:
        #    vbs = "-"

        if enable_vbs:
            table_data.append([name, num_rows, timeouts, total_runtime, par_10, vbs])
        else:
            table_data.append([name, num_rows, timeouts, total_runtime, par_10])

    if enable_vbs:
        table_df = pd.DataFrame(table_data, columns=["Algorithm", "N", "#TO", "RT", "PAR10", "#VBS"])
    else:
        table_df = pd.DataFrame(table_data, columns=["Algorithm", "N", "#TO", "RT", "PAR10"])

    table_df.sort_values(["#TO", "RT"], inplace=True)
    table_df.index = np.arange(1, len(table_df)+1)
    table_df.index.name = "No."
    table_df.reset_index(inplace=True)
    
    # Save table to file
    table_df.to_latex(output_file, index=False, float_format="%.2f")