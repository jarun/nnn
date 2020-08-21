#!/usr/bin/env python3
#
# Usage: ./plot-bench.py datafile
# (where datafile is the output of benchmark.sh)

import matplotlib.pyplot as plt
import sys

def bench_file_to_lists(infile):
    return [[float(entry) for entry in line.split('\t')[1:]] for line in infile.readlines()]

def plot_data(data):
    fig = plt.figure()
    ax = fig.add_axes([0,0,1,1])
    ax.violinplot(data)
    plt.savefig("plot.svg")

filename = sys.argv[1]

plot_data(bench_file_to_lists(open(filename)))
