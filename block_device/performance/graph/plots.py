import argparse
import math
import os

import matplotlib.pyplot as plt
import pandas as pd


MACHINES = {
    'anthem': {
        '/dev/nvme0n1': {'name': 'Samsung SSD 990 PRO (2 TB)', 'color': 'tab:blue', 'marker': 'v'},
    },
    'spartan': {
        '/dev/nvme0n1': {'name': 'Samsung SSD 970 EVO Plus (2 TB)', 'color': 'tab:orange', 'marker': '^'},
        '/dev/sda': {'name': 'Samsung SSD 860 (1 TB)', 'color': 'tab:green', 'marker': '1'},
        '/dev/sdb': {'name': 'Samsung SSD 860 (500 GB)', 'color': 'tab:red', 'marker': '2'},
    },
    'zudomon': {
        '/dev/nvme0n1': {'name': 'Samsung SSD 970 PRO (1 TB)', 'color': 'tab:purple', 'marker': '<'},
        '/dev/sda': {'name': 'Samsung SSD 860 (1 TB)', 'color': 'tab:brown', 'marker': '3'},
        '/dev/sdb': {'name': 'ST1000VX000-1ES1 (1 TB)', 'color': 'tab:pink', 'marker': 's'},
        '/dev/sdc': {'name': 'ST1000VX000-1ES1 (1 TB)', 'color': 'tab:grey', 'marker': 'D'},
    },
}


parser = argparse.ArgumentParser(
    description='Visualize benchmark results.'
)
parser.add_argument('input', metavar='INPUT', nargs='+',
                    help='list of path to the benchmark results')
parser.add_argument('output', metavar='OUTPUT',
                    help='path where to store data visualizations')
args = parser.parse_args()

data = args.input
destination = args.output

for path in data:
    print(f'[*] Read {path}')
    df = pd.read_csv(path)
    df['size_in_mib'] = df['size'] // 2**20

    # Summary statistics about the execution time
    res = df[['device', 'size_in_mib', 'time']].groupby(['device', 'size_in_mib']).describe()
    print(f'{res}\n')

    hostname, _ = os.path.splitext(os.path.basename(path))

    # Box plots
    devices = df['device'].unique()
    for device in devices:
        one_device = df[df['device'] == device]

        device_name = os.path.basename(device)

        # Group by sizes is a problem due to how the time scale grows with the
        # size of the extracted bytes
        one_device.boxplot(column=['time'], by=['size_in_mib'])
        device_name = os.path.basename(device)
        plt.savefig(f'{destination}/boxplots-{hostname}-{device_name}.pdf',
                    bbox_inches='tight', pad_inches=0)
        plt.close()

        # # The outliers are still a problem
        # sizes = one_device['size_in_mib'].unique()
        # for size in sizes:
        #     one_size = one_device[one_device['size_in_mib'] == size]

        #     one_size.boxplot(column=['time'])
        #     plt.savefig(f'{destination}/boxplots-{hostname}-{device_name}-{size}.pdf',
        #                 bbox_inches='tight', pad_inches=0)
        #     plt.close()

    # Classic plots comparing all the devices (one per machine)
    devices = df['device'].unique()
    for device in devices:
        one_device = df[df['device'] == device]
        line = one_device.groupby('size_in_mib').agg({'time': ['count', 'mean', 'std']})
        xs = line.index
        ys = line['time']['mean'].values
        errors = [1.960 * s/math.sqrt(c)
                  for c, s in zip(line['time']['count'].values,
                                  line['time']['std'].values)]
        plt.errorbar(xs, ys, yerr=errors, capsize=3,
                     color=MACHINES[hostname][device]['color'],
                     marker=MACHINES[hostname][device]['marker'], markersize=8)

    plt.xlabel('Size [MiB]')
    plt.xscale('log')
    plt.ylabel('Average time [ms]')
    plt.ylim(bottom=0)
    # plt.yscale('log')
    plt.savefig(f'{destination}/plot-{hostname}.pdf',
                bbox_inches='tight', pad_inches=0)
    plt.close()
