import os
import sys
import numpy as np
import pandas as pd

def count_lines_in_tgf_files(directory, name):
    if not os.path.isdir(directory):
        print(f"Error: {directory} is not a valid directory.")
        return
    
    df = pd.DataFrame(columns=["Instance", "#Args", "#Attacks", "AVG DEG", "MED DEG", "AVG IN", "MED IN", "AVG OUT", "MED OUT"])
    
    for root, _, files in os.walk(directory):
        for filename in files:
            if filename.endswith(".tgf"):
                file_path = os.path.join(directory, filename)
                try:
                    with open(file_path, 'r', encoding='utf-8') as file:
                        n_arguments = 0
                        n_attacks = 0
                        attack_mode = False
                        arguments_deg = {}
                        arguments_in = {}
                        arguments_out = {}
                        
                        for line in file:
                            if line.strip() == "#":
                                attack_mode = True
                                continue

                            if not attack_mode:
                                n_arguments += 1
                                arguments_deg[line.strip()] = 0
                                arguments_in[line.strip()] = 0
                                arguments_out[line.strip()] = 0
                            else:
                                att1 = line.strip().split(" ")[0]
                                att2 = line.strip().split(" ")[1]
                                arguments_in[att2] = arguments_in[att2] + 1
                                arguments_out[att1] = arguments_out[att1] + 1
                                arguments_deg[att2] = arguments_deg[att2] + 1
                                arguments_deg[att2] = arguments_deg[att2] + 1
                                n_attacks += 1

                        instance = {"Instance": filename, "#Args": n_arguments, "#Attacks": n_attacks, 
                                    "AVG DEG": np.average(list(arguments_deg.values())),
                                    "MED DEG": np.median(list(arguments_deg.values())),
                                    "AVG IN": np.average(list(arguments_in.values())),
                                    "MED IN": np.median(list(arguments_in.values())),
                                    "AVG OUT": np.average(list(arguments_out.values())),
                                    "MED OUT": np.median(list(arguments_out.values())),
                                    }
                        
                        df = df._append(instance, ignore_index=True)
                except IndexError as e:
                    print(f"Error reading {filename}: {e}")

    #print(df)
    df.to_csv(name + ".csv", index=False)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <directory> <name>")
    else:
        count_lines_in_tgf_files(sys.argv[1], sys.argv[2])
