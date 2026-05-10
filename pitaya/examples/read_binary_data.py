import struct
import numpy as np

def read_pitaya_binary(file_path):
    """
    Reads simulation data exported from Pitaya's data_exporter.
    Returns a dictionary of {signal_name: numpy_array}.
    """
    with open(file_path, 'rb') as f:
        # Header
        magic = f.read(4)
        if magic != b'PTYA':
            raise ValueError("Not a valid Pitaya binary file")
        
        version, = struct.unpack('I', f.read(4))
        num_signals, = struct.unpack('I', f.read(4))
        num_samples, = struct.unpack('I', f.read(4))
        
        # Metadata
        signals = []
        for _ in range(num_signals):
            name_len, = struct.unpack('I', f.read(4))
            name = f.read(name_len).decode('utf-8')
            dimension, = struct.unpack('I', f.read(4))
            signals.append({'name': name, 'dimension': dimension})
            
        # Data
        data = {}
        for sig in signals:
            count = num_samples * sig['dimension']
            raw_data = struct.unpack(f'{count}f', f.read(count * 4))
            
            # Reshape multi-dimensional signals
            arr = np.array(raw_data)
            if sig['dimension'] > 1:
                arr = arr.reshape((num_samples, sig['dimension']))
            
            data[sig['name']] = arr
            
    return data

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python read_binary_data.py <file.bin>")
        sys.exit(1)
        
    try:
        data = read_pitaya_binary(sys.argv[1])
        print(f"Loaded {len(data)} signals:")
        for name, arr in data.items():
            print(f"  - {name}: shape {arr.shape}")
            
        # Example plotting (requires matplotlib)
        # import matplotlib.pyplot as plt
        # if 'time' in data:
        #     for name, arr in data.items():
        #         if name != 'time':
        #             plt.plot(data['time'], arr, label=name)
        #     plt.legend()
        #     plt.show()
            
    except Exception as e:
        print(f"Error: {e}")
