import struct
import numpy as np
import os

class PitayaDataLoader:
    """
    A class to load and access simulation data exported by the Pitaya library.
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
        """Loads the binary data from the file."""
        if not os.path.exists(self.file_path):
            raise FileNotFoundError(f"File not found: {self.file_path}")

        with open(self.file_path, 'rb') as f:
            # Header
            magic = f.read(4)
            if magic != b'PTYA':
                raise ValueError("Not a valid Pitaya binary file")
            
            self.version, = struct.unpack('I', f.read(4))
            if self.version not in [1, 2]:
                raise ValueError(f"Unsupported Pitaya binary version: {self.version}")

            self.num_signals, = struct.unpack('I', f.read(4))
            self.num_samples, = struct.unpack('I', f.read(4))
            
            # Metadata
            signals = []
            for _ in range(self.num_signals):
                # Name
                name_len, = struct.unpack('I', f.read(4))
                name = f.read(name_len).decode('utf-8')
                
                group = "General"
                row = 0
                col = 0
                
                if self.version >= 2:
                    # Group
                    group_len, = struct.unpack('I', f.read(4))
                    group = f.read(group_len).decode('utf-8')
                    # Row/Col
                    row, = struct.unpack('I', f.read(4))
                    col, = struct.unpack('I', f.read(4))

                # Dimension
                dimension, = struct.unpack('I', f.read(4))
                
                sig_meta = {
                    'name': name, 
                    'group': group, 
                    'row': row, 
                    'col': col, 
                    'dimension': dimension
                }
                signals.append(sig_meta)
                self.metadata[name] = sig_meta
                
            # Data
            for sig in signals:
                count = self.num_samples * sig['dimension']
                raw_data = struct.unpack(f'{count}f', f.read(count * 4))
                
                arr = np.array(raw_data)
                if sig['dimension'] > 1:
                    arr = arr.reshape((self.num_samples, sig['dimension']))
                
                self.data[sig['name']] = arr

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
