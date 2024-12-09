import argparse

import pandas as pd


parser = argparse.ArgumentParser(
    description='Visualize benchmark results.'
)
parser.add_argument('input', metavar='INPUT', nargs='+',
                    help='list of path to the benchmark results')
# parser.add_argument('output', metavar='OUTPUT',
#                     help='path where to store data visualizations')
args = parser.parse_args()

data = args.input
# destination = args.output

for path in data:
    print(f"[*] Read {path}")
    df = pd.read_csv(path)

    res = df[["device", "size", "time"]].groupby(["device", "size"]).describe()

    print(f"{res}\n")

    # TODO: Box plots (one per device)    
    # TODO: Classic plots comparing all the devices (one per machine)
