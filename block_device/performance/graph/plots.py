#!/usr/bin/env python

import argparse
import math
import os

import matplotlib
import matplotlib.pyplot as plt
import pandas as pd

matplotlib.rc('font', size=18, family=['NewComputerModern08', 'sans-serif'])
matplotlib.rc('pdf', fonttype=42)

MACHINES = {
    'anthem': {
        '/dev/sda': {'name': 'HDD SATA', 'color': 'tab:orange', 'marker': 's'}, # WDC WD10EZEX-08M (1 TB)
        '/dev/sdb': {'name': 'SSD SATA', 'color': 'tab:purple', 'marker': '1'}, # Samsung SSD 870 (1 TB)
        '/dev/nvme0n1': {'name': 'SSD M.2 NVMe', 'color': 'tab:blue', 'marker': 'v'}, # Samsung SSD 990 PRO (2 TB)
    },
    'spartan': {
        # NOTE: Device are ordered by their perforamance profile (slowest to fastest)
        '/dev/sda': {'name': 'SSD SATA', 'color': 'indigo', 'marker': '2'}, # Samsung SSD 860 (1 TB)
        '/dev/sdb': {'name': 'SSD SATA', 'color': 'mediumpurple', 'marker': '3'}, # Samsung SSD 860 (500 GB)
        '/dev/nvme0n1': {'name': 'SSD M.2 NVMe', 'color': 'dodgerblue', 'marker': '^'}, # Samsung SSD 970 EVO+ (2 TB)
    },
    'zudomon': {
        # NOTE: Device are ordered by their perforamance profile (slowest to fastest)
        '/dev/sdb': {'name': 'HDD SATA', 'color': 'darkorange', 'marker': 'D'}, # ST1000VX000-1ES1 (1 TB)
        '/dev/sdc': {'name': 'HDD SATA', 'color': 'peachpuff', 'marker': 'd'}, # ST1000VX000-1ES1 (1 TB)
        '/dev/sda': {'name': 'SSD SATA', 'color': 'violet', 'marker': '4'}, # Samsung SSD 860 (1 TB)
        '/dev/nvme0n1': {'name': 'SSD M.2 NVMe', 'color': 'deepskyblue', 'marker': '<'}, # Samsung SSD 970 PRO (1 TB)
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
    devices = MACHINES[hostname].keys() # df['device'].unique()
    for device in devices:
        one_device = df[df['device'] == device]

        device_name = os.path.basename(device)

        # Group by sizes is a problem due to how the time scale grows with the
        # size of the extracted bytes
        one_device.boxplot(column=['time'], by=['size_in_mib'])
        plt.grid(which="major", linestyle='dashed', linewidth=0.3)
        plt.grid(which="minor", linestyle='dotted', linewidth=0.1)
        plt.suptitle(f'Hostname: {hostname}')
        plt.title(MACHINES[hostname][device]['name'])
        plt.xlabel('Key size [MiB]')
        plt.ylabel('Time [ms]')
        plt.yscale('log')
        plt.tight_layout(rect=[0, 0, 1, 1.09])
        plt.savefig(f'{destination}/boxplots-{hostname}-{device_name}.pdf',
                    bbox_inches='tight', pad_inches=0)
        plt.close()

        # Alternative without groups
        sizes = one_device['size_in_mib'].unique()
        for size in sizes:
            one_size = one_device[one_device['size_in_mib'] == size]

            one_size.boxplot(column=['time'])
            plt.grid(visible=False)
            plt.suptitle(f'Hostname: {hostname}')
            plt.title(MACHINES[hostname][device]['name'])
            plt.xlabel('Key size [MiB]')
            plt.xticks(ticks=[1], labels=[str(size)])
            plt.ylabel('Time [ms]')
            plt.tight_layout(rect=[0, 0, 1, 1.09])
            plt.savefig(f'{destination}/boxplots-{hostname}-{device_name}-{size}.pdf',
                        bbox_inches='tight', pad_inches=0)
            plt.close()

    # Classic plots comparing all the devices (one per machine)
    handles = []
    devices = MACHINES[hostname].keys() # df['device'].unique()
    for device in devices:
        if device not in MACHINES[hostname]:
            continue

        one_device = df[df['device'] == device]
        line = one_device.groupby('size_in_mib').agg({'time': ['count', 'mean', 'std']})
        xs = line.index
        ys = line['time']['mean'].values
        errors = [1.960 * s/math.sqrt(c)
                  for c, s in zip(line['time']['count'].values,
                                  line['time']['std'].values)]
        handle = plt.errorbar(xs, ys, yerr=errors, capsize=3,
                               color=MACHINES[hostname][device]['color'],
                               marker=MACHINES[hostname][device]['marker'], markersize=8)
        handles.append(handle[0]) # remove the errorbar

    labels = [MACHINES[hostname][device]['name'] for device in devices]
    x0, y0, width, height = -0.04, 1.02, 1.08, 0.2
    legend = plt.legend(handles, labels, handlelength=1.3, loc='upper left', prop={'size': 16})

    plt.xlabel('Key size [MiB]')
    plt.xscale('log')
    plt.ylabel('Average time [ms]')
    plt.yscale('log')
    bottom, top = plt.gca().get_ylim()
    plt.ylim(bottom=bottom)#, top=10**(len(labels) - 1) * top)
    plt.savefig(f'{destination}/plot-{hostname}.pdf',
                bbox_inches='tight', pad_inches=0)
    plt.close()
