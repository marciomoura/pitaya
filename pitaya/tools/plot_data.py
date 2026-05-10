#!/usr/bin/env python3
import argparse
import matplotlib.pyplot as plt
from pitaya_data import PitayaDataLoader
import sys

def main():
    parser = argparse.ArgumentParser(description="Plot Pitaya simulation binary data.")
    parser.add_argument("file", help="The binary data file (.bin) to plot.")
    parser.add_argument("-s", "--signals", nargs="+", help="Specific signals to plot. If omitted, all signals except 'time' are plotted.")
    parser.add_argument("--list", action="store_true", help="List all signals in the file and exit.")
    
    args = parser.parse_args()

    try:
        loader = PitayaDataLoader(args.file)
    except Exception as e:
        print(f"Error loading file: {e}")
        sys.exit(1)

    if args.list:
        print(f"Signals in {args.file}:")
        for name in loader.get_signal_names():
            dim = loader.metadata[name]['dimension']
            print(f"  - {name} (dim={dim})")
        sys.exit(0)

    time = loader.get_time()
    signal_names = args.signals if args.signals else [n for n in loader.get_signal_names() if n != 'time']

    plt.figure(figsize=(10, 6))
    
    for name in signal_names:
        data = loader.get_signal(name)
        if data is None:
            print(f"Warning: Signal '{name}' not found.")
            continue
            
        if time is not None:
            plt.plot(time, data, label=name)
        else:
            plt.plot(data, label=name)

    plt.title(f"Simulation Data: {args.file}")
    plt.xlabel("Time [s]" if time is not None else "Samples")
    plt.ylabel("Value")
    plt.grid(True)
    plt.legend()
    plt.show()

if __name__ == "__main__":
    main()
