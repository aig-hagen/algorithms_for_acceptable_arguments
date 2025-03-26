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

    dataframe = dataframe.astype({'runtime': 'float'})

    # Group by 'group' and extract the 'runtime' for each group
    solvers = ["IAQ-cgss2", "EEE-cgss2", "SEE-cgss2", "SEEM-cgss2"]
    new_df = pd.DataFrame()
    for slv in solvers:
        partial = dataframe[dataframe["solver_name"] == slv]
        print(partial["runtime"])
        new_df[slv] = partial["runtime"].tolist()

    new_df['contributor'] = new_df.idxmin(axis=1)
    new_df['VBS'] = new_df[["IAQ-cgss2", "EEE-cgss2", "SEE-cgss2", "SEEM-cgss2"]].min(axis=1)

    print(new_df)

    table_data = []

    solvers.append("VBS")

    for name in solvers:
        num_rows = len(new_df[name])
        timeouts = new_df[name].eq(timeout).sum()
        total_runtime = new_df[name].sum() - (timeouts* timeout)
        par_10 = (new_df[name].sum() + (9 * timeouts * timeout)) / num_rows
        if name != "VBS":
            vbs = new_df["contributor"].value_counts().get(name, 0)
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

    """
    # Group by solver
    grouped_dataframe = dataframe.groupby("solver_name")
    
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
    """
    
    # Save table to file
    table_df.to_latex(output_file.replace('.pgf', '_table.tex').replace('.png', '_table.tex'), index=False, float_format="%.2f")