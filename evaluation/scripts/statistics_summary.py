import sys
import pandas as pd

def summarise_stats(file):
    df = pd.read_csv(file + ".csv")

    df_results = pd.DataFrame(columns=["Dataset", "AVG ARGS", "MED ARGS", "STD ARGS", "MIN ARGS", "MAX ARGS", "AVG ATTS", "MED ATTS", "STD ATTS", "MIN ATTS", "MAX ATTS", "AVG DEG"])
    row = {"Dataset": file, 
           "AVG ARGS": df["#Args"].mean(),"MED ARGS": df["#Args"].median(), 
           "STD ARGS": df["#Args"].std(),
           "MIN ARGS": df["#Args"].min(),"MAX ARGS": df["#Args"].max(), 
           "AVG ATTS": df["#Attacks"].mean(), "MED ATTS": df["#Attacks"].median(),
           "STD ATTS": df["#Attacks"].std(),
           "MIN ATTS": df["#Attacks"].min(), "MAX ATTS": df["#Attacks"].max(),
           "AVG DEG": df["AVG DEG"].mean(),
           }
    df_results = df_results._append(row, ignore_index=True)
    print(df)

    print(df_results)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <file>")
    else:
        summarise_stats(sys.argv[1])