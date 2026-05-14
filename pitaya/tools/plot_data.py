#!/usr/bin/env python3
import argparse
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from pitaya_data import PitayaDataLoader
import sys
import webbrowser
import os

def parse_signal_name(name):
    """Fallback parsing if explicit group/pos isn't used."""
    if ":" in name:
        parts = name.split(":", 1)
        return parts[0].strip(), parts[1].strip()
    return "General", name

def main():
    parser = argparse.ArgumentParser(description="Plot Pitaya simulation binary data using Plotly.")
    parser.add_argument("file", help="The binary data file (.bin) to plot.")
    parser.add_argument("-o", "--output", help="Path to save the output HTML file. Defaults to <file>.html")
    parser.add_argument("--decimate", type=int, default=1, help="Decimation factor (plot every Nth point).")
    parser.add_argument("--open", action="store_true", help="Open the report in the browser automatically.")
    
    args = parser.parse_args()

    try:
        loader = PitayaDataLoader(args.file)
    except Exception as e:
        print(f"Error loading file: {e}")
        sys.exit(1)

    time = loader.get_time()
    signal_names = [n for n in loader.get_signal_names() if n != 'time']

    # Organize signals by (row, col)
    # Grid is 1-based index (Plotly convention)
    grid = {}
    max_row = 1
    max_col = 1

    # First pass: map explicit positions
    auto_signals = []
    groups = {} # Group name -> list of signals

    for name in signal_names:
        meta = loader.metadata[name]
        group = meta['group']
        row = meta['row']
        col = meta['col']

        # Fallback to name parsing if metadata is default
        if group == "General" and ":" in name:
            group, display_name = parse_signal_name(name)
        else:
            display_name = name

        if row > 0 and col > 0:
            pos = (row, col)
            if pos not in grid: grid[pos] = []
            grid[pos].append((display_name, name))
            max_row = max(max_row, row)
            max_col = max(max_col, col)
        else:
            if group not in groups: groups[group] = []
            groups[group].append((display_name, name))

    # Second pass: map grouped signals to rows
    # We assign each group to its own row if not explicitly positioned
    next_row = max_row + (1 if grid else 0)
    for group in sorted(groups.keys()):
        pos = (next_row, 1)
        grid[pos] = groups[group]
        max_row = next_row
        next_row += 1

    # Create subplots
    fig = make_subplots(
        rows=max_row, 
        cols=max_col,
        shared_xaxes=True,
        vertical_spacing=0.05,
        subplot_titles=[f"Subplot {r},{c}" for r, c in sorted(grid.keys())] # Temporary titles
    )

    # Add traces
    dec = args.decimate
    for (row, col), signals in grid.items():
        for display_name, raw_name in signals:
            data = loader[raw_name]
            dim = loader.metadata[raw_name]['dimension']
            
            t = time[::dec] if time is not None else None
            
            if dim > 1:
                for d in range(dim):
                    y = data[::dec, d]
                    fig.add_trace(
                        go.Scatter(x=t, y=y, name=f"{display_name}[{d}]", mode='lines'),
                        row=row, col=col
                    )
            else:
                y = data[::dec]
                fig.add_trace(
                    go.Scatter(x=t, y=y, name=display_name, mode='lines'),
                    row=row, col=col
                )

    fig.update_layout(
        template="plotly_dark",
        title=f"Pitaya Simulation: {os.path.basename(args.file)}",
        height=400 * max_row,
        hovermode='x unified'
    )

    output_path = args.output if args.output else args.file + ".html"
    fig.write_html(output_path)
    print(f"Report generated: {output_path}")

    if args.open:
        webbrowser.open('file://' + os.path.realpath(output_path))

if __name__ == "__main__":
    main()
