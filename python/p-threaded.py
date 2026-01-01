import hashlib
import threading
import os
import time

# Scanning directory tree to find filename, size, mtime, owner and hash
def initial_scan(directory):
    print(f"\nScanning {directory} for files")
    start_time = time.time()

    files = []
    for dirpath, dirnames, filenames in os.walk(directory):
        for filename in filenames:
            files.append(os.path.join(dirpath, filename))
    
    end_time = time.time()
    print(f"Found {len(files)} files in {end_time - start_time:.2f} seconds")
    return files

# Getting file sizes
def file_sizes(files):
    print(f"\nGetting file sizes for each file in {directory}")
    start_time = time.time()
    sizes = []
    for file in files:
        try:
            sizes.append((file, os.path.getsize(file)))
        except Exception as e:
            print(f"Error getting size for {file}: {e}")
    end_time = time.time()
    print(f"Got sizes for {len(sizes)} files in {end_time - start_time:.2f} seconds")
    return sizes

# Adding simple CLI queries

# Adding hash algorithm selection


if __name__ == "__main__":
    directory = input("Enter directory to scan: ").strip()
    if not directory:
        print("No directory provided")
        exit(1)
    if not os.path.isabs(directory):
        directory = os.path.expanduser('~/' + directory)
    
    files = initial_scan(directory)
    sizes = file_sizes(files)
    print("Sample of 10 file sizes:")
    for file, size in sizes[:5]:
        print(f"  {file}: {size} bytes")