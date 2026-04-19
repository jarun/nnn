#!/usr/bin/env python3
#
# Usage: ./plot-bench.py datafile
# (where datafile is the output of benchmark.sh)

import matplotlib.pyplot as plt
import sys


def bench_file_to_lists(infile):
    """Read benchmark output file and convert to list of float lists.

    Args:
        infile: File object to read from.

    Returns:
        List of lists, where each inner list contains timing values.
    """
    return [[float(entry) for entry in line.split('\t')[1:]] for line in infile.readlines()]


def plot_data(data):
    """Generate violin plot from benchmark data.

    Args:
        data: List of lists containing benchmark timing values.
    """
    fig = plt.figure()
    ax = fig.add_axes([0, 0, 1, 1])
    ax.violinplot(data)
    plt.savefig("plot.svg")


def main():
    """Main entry point for the benchmark plotting script."""
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} datafile", file=sys.stderr)
        sys.exit(1)

    filename = sys.argv[1]

    try:
        with open(filename) as infile:
            plot_data(bench_file_to_lists(infile))
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found", file=sys.stderr)
        sys.exit(1)
    except ValueError as e:
        print(f"Error: Invalid data format in '{filename}': {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
