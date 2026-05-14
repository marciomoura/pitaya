import csv
import numpy as np
import os
import re

class PitayaDataLoader:
    """
    A class to load and access simulation data exported by the Pitaya library in CSV format.
    """
    def __init__(self, file_path):
        self.file_path = file_path
        self.metadata = {}
        self.data = {}
        self.num_samples = 0
        self.num_signals = 0
        self.version = 0
        self.load()

    def load(self):
        """Loads the CSV data from the file."""
        if not os.path.exists(self.file_path):
            raise FileNotFoundError(f"File not found: {self.file_path}")

        with open(self.file_path, 'r') as f:
            lines = f.readlines()

        # Parse Metadata from comments
        signal_metas = []
        data_start_line = 0
        
        for i, line in enumerate(lines):
            if not line.startswith('#'):
                data_start_line = i
                break
            
            if "PTYA_CSV_VERSION" in line:
                self.version = int(line.split(':')[-1].strip())
            
            if "SIGNAL:" in line:
                # Use regex to parse: # SIGNAL: name=sine, group=General, row=0, col=0, dim=1
                match = re.search(r"name=(?P<name>[^,]+),\s*group=(?P<group>[^,]+),\s*row=(?P<row>\d+),\s*col=(?P<col>\d+),\s*dim=(?P<dim>\d+)", line)
                if match:
                    meta = {
                        'name': match.group('name').strip(),
                        'group': match.group('group').strip(),
                        'row': int(match.group('row')),
                        'col': int(match.group('col')),
                        'dimension': int(match.group('dim'))
                    }
                    signal_metas.append(meta)
                    self.metadata[meta['name']] = meta

        self.num_signals = len(signal_metas)
        
        # Parse column names and data
        reader = csv.DictReader(lines[data_start_line:])
        
        # Initialize data lists
        temp_data = {meta['name']: [] for meta in signal_metas}
        
        for row in reader:
            self.num_samples += 1
            for meta in signal_metas:
                name = meta['name']
                dim = meta['dimension']
                
                if dim == 1:
                    temp_data[name].append(float(row[name]))
                else:
                    # Multi-dimensional signal is stored as name_0, name_1, ...
                    sample = [float(row[f"{name}_{d}"]) for d in range(dim)]
                    temp_data[name].append(sample)

        # Convert to numpy arrays
        for name, values in temp_data.items():
            self.data[name] = np.array(values)

    def get_signal(self, name):
        """Returns the data for a given signal name."""
        return self.data.get(name)

    def get_signal_names(self):
        """Returns a list of all signal names."""
        return list(self.data.keys())

    def get_time(self):
        """Returns the 'time' signal if present, otherwise None."""
        return self.get_signal('time')

    def __getitem__(self, key):
        return self.get_signal(key)

    def __contains__(self, key):
        return key in self.data

    def __repr__(self):
        return f"PitayaDataLoader(file='{self.file_path}', signals={self.get_signal_names()}, samples={self.num_samples})"
