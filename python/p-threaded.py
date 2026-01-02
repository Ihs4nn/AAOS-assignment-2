import hashlib
import threading
import os
import time
import pwd
import csv

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

# Getting file names
def file_names(files):
    print(f"\nGetting file names for each file in {directory}")
    start_time = time.time()
    names = [os.path.basename(file) for file in files]
    end_time = time.time()
    print(f"Got names for {len(names)} files in {end_time - start_time:.2f} seconds")
    return names

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

# Getting file mtimes
def file_mtimes(files):
    print(f"\nGetting file modification times for each file in {directory}")
    start_time = time.time()
    mtimes = []
    for file in files:
        try:
            mtimes.append((file, os.path.getmtime(file)))
        except Exception as e:
            print(f"Error getting mtime for {file}: {e}")
    end_time = time.time()
    print(f"Got mtimes for {len(mtimes)} files in {end_time - start_time:.2f} seconds")
    return mtimes

# Getting file owner names
def file_owners(files):
    print(f"\nGetting file owners for each file in {directory}")
    start_time = time.time()
    owners = []
    for file in files:
        try:
            stat_info = os.stat(file)
            owner_name = pwd.getpwuid(stat_info.st_uid).pw_name
            owners.append((file, owner_name))
        except (OSError, KeyError) as e:
            print(f"Error getting owner for {file}: {e}")
            owners.append((file, "unknown"))
    end_time = time.time()
    print(f"Got owners for {len(owners)} files in {end_time - start_time:.2f} seconds")
    return owners

# Getting file hashes
def file_hash(files, algorithm='sha256'):
    print(f"\nCalculating {algorithm} hashes for each file in {directory}")
    start_time = time.time()
    hashes = []
    for file in files:
        try:
            hash_obj = hashlib.new(algorithm)
            with open(file, 'rb') as f:
                while chunk := f.read(8192):
                    hash_obj.update(chunk)
            hashes.append((file, hash_obj.hexdigest()))
        except Exception as e:
            print(f"Error hashing {file}: {e}")
            hashes.append((file, None))
    end_time = time.time()
    print(f"Calculated hashes for {len(hashes)} files in {end_time - start_time:.2f} seconds")
    return hashes

# Writing infomation to CSV file
def write_to_csv(files, sizes, mtimes, owners, hashes):
    index = []
    for i in range(len(files)):
        index.append({
            'filename': files[i],
            'size': sizes[i][1] if i < len(sizes) and sizes[i][0] == files[i] else None,
            'mtime': mtimes[i][1] if i < len(mtimes) and mtimes[i][0] == files[i] else None,
            'owner': owners[i][1] if i < len(owners) and owners[i][0] == files[i] else None,
            'hash': hashes[i][1] if i < len(hashes) and hashes[i][0] == files[i] else None
        })
    # Write to CSV
    csv_file = '../results/p-threaded-file-index.csv'
    with open(csv_file, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=['filename', 'size', 'mtime', 'owner', 'hash'])
        writer.writeheader()
        writer.writerows(index)
    print(f"Saved index to {csv_file} with {len(index)} entries")

# Adding simple CLI queries
def query_large_files():
    pass

def query_checksum_files():
    pass

# Adding hash algorithm selection
def change_hash_algorithm():
    pass

# Main script
if __name__ == "__main__":
    # Get choice of hash algorithm
    print("\nSelect hash algorithm:")
    print("1. sha-256 (default algorithm)")
    print("2. sha-1")
    print("3. md5")
    choice = input("Enter choice (1-3): ").strip()

    if choice == '1':
        hash_algorithm = 'sha256'
    elif choice == '2':
        hash_algorithm = 'sha1'
    else:
        hash_algorithm = 'md5'

    print(f"\nUsing {hash_algorithm} as hash algorithm")

    # Get directory from user
    directory = input("\nEnter directory to scan: ").strip()
    if not directory:
        print("No directory provided")
        exit(1)
    if not os.path.isabs(directory):
        directory = os.path.expanduser('~/' + directory)
    
    files = initial_scan(directory)
    names = file_names(files)
    sizes = file_sizes(files)
    mtimes = file_mtimes(files)
    owners = file_owners(files)
    hashes = file_hash(files, algorithm = hash_algorithm)
    print("Writing all files infomation to CSV file")
    write_to_csv(files, sizes, mtimes, owners, hashes)

    