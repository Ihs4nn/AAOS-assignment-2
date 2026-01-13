import hashlib
import os
import time
import pwd
import csv
import resource

# Scanning directory tree to find filename, size, mtime, owner and hash
def initial_scan(directory):
    print(f"\nScanning {directory} for files")
    wall_start = time.time()
    cpu_start = time.process_time()

    files = []
    for dirpath, dirnames, filenames in os.walk(directory):
        for filename in filenames:
            files.append(os.path.join(dirpath, filename))
    
    wall_end = time.time()
    cpu_end = time.process_time()
    print(f"Found {len(files)} files in {wall_end - wall_start:.2f} real seconds and {cpu_end - cpu_start:.2f} CPU seconds")
    return files

# Getting file names
def file_names(files):
    print(f"\nGetting file names for each file in {directory}")
    wall_start = time.time()
    cpu_start = time.process_time()

    from concurrent.futures import ThreadPoolExecutor, as_completed
    def get_name(file):
        return os.path.basename(file)
    names = []
    with ThreadPoolExecutor(max_workers=3) as executor:
        future_to_file = {executor.submit(get_name, file): file for file in files}
        for future in as_completed(future_to_file):
            names.append(future.result())

    wall_end = time.time()
    cpu_end = time.process_time()
    print(f"Got names for {len(names)} files in {wall_end - wall_start:.2f} real seconds and {cpu_end - cpu_start:.2f} CPU seconds")
    return names

# Getting file sizes
def file_sizes(files):
    print(f"\nGetting file sizes for each file in {directory}")
    wall_start = time.time()
    cpu_start = time.process_time()
    def get_size(file):
        try:
            return (file, os.path.getsize(file))
        except Exception as e:
            return (file, None)
    sizes = []
    from concurrent.futures import ThreadPoolExecutor, as_completed
    # Create a threadpool with 3 threads
    with ThreadPoolExecutor(max_workers=3) as executor:
        # Submit each file to the thread pool for processing
        future_to_file = {executor.submit(get_size, file): file for file in files}
        # Collect results as they finish
        for future in as_completed(future_to_file):
            sizes.append(future.result())
    wall_end = time.time()
    cpu_end = time.process_time()
    print(f"Got sizes for {len(sizes)} files in {wall_end - wall_start:.2f} real seconds and {cpu_end - cpu_start:.2f} CPU seconds")
    return sizes

# Getting file mtimes
def file_mtimes(files):
    print(f"\nGetting file modification times for each file in {directory}")
    wall_start = time.time()
    cpu_start = time.process_time()
    # Submitting all files to thread pool
    from concurrent.futures import ThreadPoolExecutor, as_completed
    def get_mtime(file):
        try:
            return (file, os.path.getmtime(file))
        except Exception as e:
            return (file, None)
    mtimes = []
    # Collecting results as they complete
    with ThreadPoolExecutor(max_workers=3) as executor:
        future_to_file = {executor.submit(get_mtime, file): file for file in files}
        for future in as_completed(future_to_file):
            mtimes.append(future.result())
    wall_end = time.time()
    cpu_end = time.process_time()
    print(f"Got mtimes for {len(mtimes)} files in {wall_end - wall_start:.2f} real seconds and {cpu_end - cpu_start:.2f} CPU seconds")
    return mtimes

# Getting file owner names
def file_owners(files):
    print(f"\nGetting file owners for each file in {directory}")
    wall_start = time.time()
    cpu_start = time.process_time()
    from concurrent.futures import ThreadPoolExecutor, as_completed
    def get_owner(file):
        try:
            stat_info = os.stat(file)
            owner_name = pwd.getpwuid(stat_info.st_uid).pw_name
            return (file, owner_name)
        except (OSError, KeyError) as e:
            return (file, "unknown")
    owners = []
    # Create 3 threads to process files at the same time
    with ThreadPoolExecutor(max_workers=3) as executor:
        future_to_file = {executor.submit(get_owner, file): file for file in files}
        # As each thread finishes, add its result to owners list
        for future in as_completed(future_to_file):
            owners.append(future.result())
    wall_end = time.time()
    cpu_end = time.process_time()
    print(f"Got owners for {len(owners)} files in {wall_end - wall_start:.2f} real seconds and {cpu_end - cpu_start:.2f} CPU seconds")
    return owners

# Getting file hashes
def file_hash(files, algorithm='sha256'):
    print(f"\nCalculating {algorithm} hashes for each file in {directory}")
    wall_start = time.time()
    cpu_start = time.process_time()
    from concurrent.futures import ThreadPoolExecutor, as_completed
    def hash_file(file):
        try:
            hash_obj = hashlib.new(algorithm)
            with open(file, 'rb') as f:
                while True:
                    chunk = f.read(8192)
                    if not chunk:
                        break
                    hash_obj.update(chunk)
            return (file, hash_obj.hexdigest())
        except Exception as e:
            return (file, None)
    hashes = []
    # Use 3 threads to calculate hashes in parallel
    with ThreadPoolExecutor(max_workers=3) as executor:
        future_to_file = {executor.submit(hash_file, file): file for file in files}
        # As each thread completes, collect its hash result in the list
        for future in as_completed(future_to_file):
            hashes.append(future.result())
    wall_end = time.time()
    cpu_end = time.process_time()
    print(f"Calculated hashes for {len(hashes)} files in {wall_end - wall_start:.2f} real seconds and {cpu_end - cpu_start:.2f} CPU seconds")
    return hashes

# Writing infomation to CSV file
def write_to_csv(files, sizes, mtimes, owners, hashes):
    print("\nWriting all files infomation to CSV file")
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
    print(f"Saved file index infomation to {csv_file} with {len(index)} entries")
    return csv_file

def get_csv_file(csv_file):
    index = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            index.append({
                'filename': row['filename'],
                'size': int(row['size']) if row['size'] else None,
                'mtime': float(row['mtime']) if row['mtime'] else None,
                'owner': row['owner'],
                'hash': row['hash'] 
            })
    return index

# Adding simple CLI queries
def query_large_files(index, mb_size):
    try:
        mb = float(mb_size)
        bytes_size = mb * 1024 * 1024
        matched_files = [f for f in index if f['size'] and f['size'] > bytes_size]
        
        total_matches = len(matched_files)
        print(f"\nFound {total_matches} files larger than {mb:.1f} MB")
        if total_matches == 0:
            return
        batch_size = 10
        for i in range(0, total_matches, batch_size):
            batch = matched_files[i : i + batch_size]
            print(f"\nDisplaying files {i + 1} to {min(i + batch_size, total_matches)} of {total_matches}")
            for file in batch:
                print(f"File: {file['filename']}, Size: {file['size'] / (1024 * 1024):.2f} MB")
            if i + batch_size < total_matches:
                user_input = input("\nType 'n' to see the next set of results or 'e' to exit:")
                if user_input.lower() == 'e':
                    break
        print("End of results")
    except ValueError:
        print("Invalid size input")

def query_checksum_files(index, checksum_name):
    for f in index:
        if os.path.basename(f['filename']) == checksum_name:
            print(f"Checksum for {checksum_name}: {f['hash']}")
            return
    print("File not found.")

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

        # Get directory from user
        directory = input("\nEnter directory to scan: ").strip()
        if not directory:
            print("No directory provided")
            exit(1)
        if not os.path.isabs(directory):
            directory = os.path.expanduser('~/' + directory)

        # Start processing with benchmarks
        print(f"Starting indexing processing using {hash_algorithm} algorithm on directory: {directory}")
        wall_start = time.time()
        cpu_start = time.process_time()
    
        files = initial_scan(directory)
        names = file_names(files)
        sizes = file_sizes(files)
        # New sort lines to match original file order because of threading
        sizes.sort(key=lambda x: files.index(x[0]))
        mtimes = file_mtimes(files)
        mtimes.sort(key=lambda x: files.index(x[0]))
        owners = file_owners(files)
        owners.sort(key=lambda x: files.index(x[0]))
        hashes = file_hash(files, algorithm = hash_algorithm)
        hashes.sort(key=lambda x: files.index(x[0]))
        csv_file = write_to_csv(files, sizes, mtimes, owners, hashes)
        wall_end = time.time()
        cpu_end = time.process_time()
        
        # AI Aided with peak memory usage calculation
        usage = resource.getrusage(resource.RUSAGE_SELF)
        peak_mem_mb = usage.ru_maxrss / (1024 * 1024) if os.uname().sysname == 'Darwin' else usage.ru_maxrss / 1024
    
        print("\nBenchmark results for Single Threaded Processing:")
        print(f"Total time taken: {wall_end - wall_start:.2f}")
        print(f"Total CPU time (Processor time used): {cpu_end - cpu_start:.2f}")
        print(f"Peak memory usage: {peak_mem_mb:.2f} MB")

        index = get_csv_file(csv_file)

        # Query menu to users
        while True:
            print("\nQuery Menu:")
            print("1. Find all files larger than x MB")
            print("2. Find the hash value of a specific file")
            print("3. Exit")
            query_choice = input("Enter 1, 2 or 3: ").strip()

            if query_choice == '1':
                mb_size = input("Enter size in MB: ").strip()
                query_large_files(index, mb_size)
            elif query_choice == '2':
                checksum_name = input("Enter file to check: ").strip()
                query_checksum_files(index, checksum_name)
            else:
                break