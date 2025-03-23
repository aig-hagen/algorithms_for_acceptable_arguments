import sys
import pandas as pd

def summarise_stats(file):
    sets = ["ICCMA15", "ICCMA17", "ICCMA19", "ICCMA21", "ICCMA23"]
    df_results = pd.DataFrame(columns=["Dataset", "#Instances", "AVG ARGS", "MED ARGS", "STD ARGS", "MIN ARGS", "MAX ARGS", "AVG ATTS", "MED ATTS", "STD ATTS", "MIN ATTS", "MAX ATTS", "AVG DEG"])

    for file in sets:
        df = pd.read_csv(file + ".csv")


        row = {"Dataset": file, "#Instances": len(df["Instance"].unique()),
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

    df_results.to_latex('table.tex', index=False, float_format="%.0f")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <file>")
    else:
        summarise_stats(sys.argv[1])