import hashlib
import threading
import os
import time

def initial_scan(directory):
    print(f"Scanning {directory} for files")
    start_time = time.time()

    files = []
    for dirpath, dirnames, filenames in os.walk(directory):
        for filename in filenames:
            files.append(os.path.join(dirpath, filename))
    
    end_time = time.time()
    print(f"Found {len(files)} files in {end_time - start_time:.2f} seconds")
    return files

if __name__ == "__main__":
    directory = input("Enter directory to scan: ").strip()
    if not directory:
        print("No directory provided")
        exit(1)
    if not os.path.isabs(directory):
        directory = os.path.expanduser('~/' + directory)
    files = initial_scan(directory)

