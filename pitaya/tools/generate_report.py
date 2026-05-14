#!/usr/bin/env python3
import argparse
import subprocess
import os
import glob
import sys
import webbrowser
import shutil

def main():
    parser = argparse.ArgumentParser(description="Cross-platform simulation report generator.")
    parser.add_argument("-e", "--executable", help="Path to the GTest executable. If omitted, skip test execution.")
    parser.add_argument("-f", "--filter", default="*", help="GTest filter (default: *)")
    parser.add_argument("-o", "--output-dir", default="reports", help="Directory to save reports.")
    parser.add_argument("-i", "--input-dir", default=".", help="Directory to search for .bin files.")
    parser.add_argument("--open", action="store_true", help="Open reports in browser (local only).")
    parser.add_argument("--headless", action="store_true", help="Run without trying to open browser (CI/Docker).")
    
    args = parser.parse_args()

    # 1. Create output directory
    if not os.path.exists(args.output_dir):
        os.makedirs(args.output_dir)

    # 2. Run GTest if executable provided
    if args.executable:
        print(f"Running tests: {args.executable} --gtest_filter={args.filter}")
        try:
            result = subprocess.run([args.executable, f"--gtest_filter={args.filter}"], capture_output=False)
            if result.returncode != 0:
                print("Tests failed!", file=sys.stderr)
        except Exception as e:
            print(f"Error running executable: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        print("No executable provided. Searching for existing .csv files...")

    # 3. Find generated CSV files
    search_path = os.path.join(args.input_dir, "**", "*.csv")
    all_csvs = glob.glob(search_path, recursive=True)
    
    csv_files = []
    for f in all_csvs:
        # Skip files already in the output directory
        if os.path.abspath(args.output_dir) in os.path.abspath(f):
            continue
        # Quick check for PTYA_CSV_VERSION magic string
        try:
            with open(f, 'r') as cf:
                first_line = cf.readline()
                if "PTYA_CSV_VERSION" in first_line:
                    csv_files.append(f)
        except Exception:
            continue

    if not csv_files:
        print(f"No valid Pitaya CSV data files found in {args.input_dir}.")
        return

    # 4. Generate Reports
    plot_script = os.path.join(os.path.dirname(__file__), "plot_data.py")
    
    for csv_file in csv_files:
        base_name = os.path.splitext(os.path.basename(csv_file))[0]
        html_file = os.path.join(args.output_dir, f"{base_name}.html")
        
        print(f"Generating report: {html_file}")
        
        plot_cmd = [sys.executable, plot_script, csv_file, "-o", html_file]
        try:
            subprocess.run(plot_cmd, check=True)
            
            if args.open and not args.headless:
                webbrowser.open('file://' + os.path.realpath(html_file))
                
            # Archive the CSV file next to the report
            target_csv = os.path.join(args.output_dir, os.path.basename(csv_file))
            if os.path.exists(target_csv):
                os.remove(target_csv)
            shutil.move(csv_file, target_csv)
            
        except Exception as e:
            print(f"Error generating plot for {csv_file}: {e}", file=sys.stderr)

    print(f"Done! Reports saved in: {os.path.abspath(args.output_dir)}")

if __name__ == "__main__":
    main()
