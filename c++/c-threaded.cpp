#include <chrono>
#include <sys/stat.h>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sys/types.h>
#include <pwd.h>
#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <openssl/sha.h>
#include <openssl/md5.h>


// Used AI and tutor skeleton to help convert python functions into C++ functions below
namespace fs = std::filesystem;

// Convert filesystem::file_time_type to seconds since epoch (best-effort)
long long fileTimeToEpochSeconds(fs::file_time_type ft) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(
        ft - fs::file_time_type::clock::now() + system_clock::now()
    );
    return duration_cast<seconds>(sctp.time_since_epoch()).count();
}
// Get file owners from directory
std::string getOwner(const fs::path& p) {
    struct stat info;
    if (stat(p.c_str(), &info) == 0) {
        struct passwd* pw = getpwuid(info.st_uid);
        if (pw) return std::string(pw->pw_name);
    }
    return "unknown";
}
// Generic hash function for sha256, sha1, md5
std::string hash_file(const fs::path& p, const std::string& algorithm) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return "";
    char buf[8192];
    std::ostringstream oss;
    // Uses sha256 based on user choice
    if (algorithm == "sha256") {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        // Reads file and then updates hash
        while (f.good()) {
            f.read(buf, sizeof(buf));
            SHA256_Update(&ctx, buf, f.gcount());
        }
        // Stores final hash value
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &ctx);
        // Converts into hex string for csv file
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        // Uses sha1 based on user choice
    } else if (algorithm == "sha1") {
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        // Does same as above but for sha1 algorithm
        while (f.good()) {
            f.read(buf, sizeof(buf));
            SHA1_Update(&ctx, buf, f.gcount());
        }
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1_Final(hash, &ctx);
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        // Uses md5 based on user choice
    } else if (algorithm == "md5") {
        MD5_CTX ctx;
        MD5_Init(&ctx);
        // Does same as above but for md5 algorithm
        while (f.good()) {
            f.read(buf, sizeof(buf));
            MD5_Update(&ctx, buf, f.gcount());
        }
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5_Final(hash, &ctx);
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}
// Threaded function 
template <typename Func, typename OutVec>
void threaded_map(const std::vector<fs::path>& files, OutVec& out, Func func, int num_threads = 3) {
    auto worker = [&](int thread_id) {
        for (size_t i = thread_id; i < files.size(); i += num_threads) {
            out[i] = func(files[i]);
        }
    };
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(worker, i);
    for (auto& t : threads) t.join();
}
// End of AI aided help

int main() {
    // Option to change hashing [Manual work]
    std::string hash_option;
    std::cout << "Select hash algorithm: "
              << "\n1. sha-256 (default)"
              << "\n2. sha-1"
              << "\n3. md5"
              << "\nEnter choice (1-3): ";
    std::getline(std::cin, hash_option);
    std::string hash_algorithm;
    if (hash_option == "1" || hash_option.empty()) {
        hash_algorithm = "sha256";
    } else if (hash_option == "2") {
        hash_algorithm = "sha1";
    } else if (hash_option == "3") {
        hash_algorithm = "md5";
    } else {
        std::cout << "Invalid choice. Using default sha-256.\n";
        hash_algorithm = "sha256";
    }
    // Start of AI aided help
    // Get input from user for directory to scan
    std::string dir;
    std::cout << "Enter directory to scan: ";
    std::getline(std::cin, dir);
    if (dir.empty()) {
        std::cerr << "No directory provided\n";
        return 1;
    }
    fs::path root = dir;
    std::string csv_file = "../results/c-single-threaded-index.csv";
    std::vector<fs::path> files;
    auto wall_start = std::chrono::steady_clock::now();
    auto cpu_start = std::clock();

    // Scan for files names
    std::cout << "\nScanning " << root << " for files... using " << hash_algorithm << "\n";
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (fs::is_regular_file(entry.path())) {
            files.push_back(entry.path());
        }
    }
    auto wall_end = std::chrono::steady_clock::now();
    auto cpu_end = std::clock();
    std::cout << "\nFound " << files.size() << " file names in "
              << std::chrono::duration<double>(wall_end - wall_start).count() << " real seconds and "
              << double(cpu_end - cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Getting file sizes (threaded)
    auto size_wall_start = std::chrono::steady_clock::now();
    auto size_cpu_start = std::clock();
    std::vector<uintmax_t> sizes(files.size());
    threaded_map(files, sizes, [](const fs::path& f) {
        try { return fs::file_size(f); } catch (...) { return uintmax_t(0); }
    });
    auto size_wall_end = std::chrono::steady_clock::now();
    auto size_cpu_end = std::clock();
    std::cout << "Found sizes for " << files.size() << " files in "
              << std::chrono::duration<double>(size_wall_end - size_wall_start).count() << " real seconds and "
              << double(size_cpu_end - size_cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Getting file mtimes (threaded)
    auto mtime_wall_start = std::chrono::steady_clock::now();
    auto mtime_cpu_start = std::clock();
    std::vector<long long> mtimes(files.size());
    threaded_map(files, mtimes, [](const fs::path& f) {
        try { return fileTimeToEpochSeconds(fs::last_write_time(f)); } catch (...) { return 0LL; }
    });
    auto mtime_wall_end = std::chrono::steady_clock::now();
    auto mtime_cpu_end = std::clock();
    std::cout << "Found mtimes for " << files.size() << " files in "
              << std::chrono::duration<double>(mtime_wall_end - mtime_wall_start).count() << " real seconds and "
              << double(mtime_cpu_end - mtime_cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Getting file owners (threaded)
    auto owner_wall_start = std::chrono::steady_clock::now();
    auto owner_cpu_start = std::clock();
    std::vector<std::string> owners(files.size());
    threaded_map(files, owners, [](const fs::path& f) {
        return getOwner(f);
    });
    auto owner_wall_end = std::chrono::steady_clock::now();
    auto owner_cpu_end = std::clock();
    std::cout << "Found owners for " << files.size() << " files in "
              << std::chrono::duration<double>(owner_wall_end - owner_wall_start).count() << " real seconds and "
              << double(owner_cpu_end - owner_cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Getting file hashes (threaded)
    auto hash_wall_start = std::chrono::steady_clock::now();
    auto hash_cpu_start = std::clock();
    std::vector<std::string> hashes(files.size());
    threaded_map(files, hashes, [&](const fs::path& f) {
        return hash_file(f, hash_algorithm);
    });
    auto hash_wall_end = std::chrono::steady_clock::now();
    auto hash_cpu_end = std::clock();
    std::cout << "Found hashes for " << files.size() << " files in "
              << std::chrono::duration<double>(hash_wall_end - hash_wall_start).count() << " real seconds and "
              << double(hash_cpu_end - hash_cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";
    // Write to CSV
    std::ofstream csv(csv_file);
    csv << "filename,size,mtime,owner,hash\n";
    for (size_t i = 0; i < files.size(); ++i) {
        csv << '"' << files[i].string() << '"' << ',' << sizes[i] << ',' << mtimes[i] << ',' << owners[i] << ',' << hashes[i] << "\n";
    }
    csv.close();
    std::cout << "\nSaved file index information to " << csv_file << " with " << files.size() << " entries\n";
    // End of AI help, rest of code is manually written

    // Printing out benchmark results and peak memory usage
    std::cout << "\nBenchmark results:\n";
    std::cout << "Total time taken: " << std::chrono::duration<double>(wall_end - wall_start).count() << " seconds\n";
    std::cout << "Total CPU time (Processor time used): " << double(cpu_end - cpu_start) / CLOCKS_PER_SEC << " seconds\n";
    // Get peak memory usage (in MB)
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    double peak_mem_mb = usage.ru_maxrss / (1024.0 * 1024.0);
    std::cout << "Peak memory usage: " << std::fixed << std::setprecision(2) << peak_mem_mb << " MB\n";
       
    // Creating interactive query menu for users:
    while (true) {
        // Printing out menu options to user
        std::cout << "\nQuery Menu:"
                  << "\n1. Find all files larger than X MB"
                  << "\n2. Find the hash value of a specific file"
                  << "\n3. Exit";
        std::cout << "\nEnter choice (1-3): ";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "1") {
            std::cout << "Enter size in MB: ";
            std::string size_input;
            std::getline(std::cin, size_input);
            // Convert MB to bytes for comparison
            uintmax_t size_threshold = std::stoull(size_input) * 1024 * 1024;
            std::cout << "Files larger than " << size_input << " MB:\n";
            for (size_t i = 0; i < files.size(); ++i) {
                if (sizes[i] > size_threshold) {
                    // Display file path name and size in MB
                    std::cout << files[i] << " (" << sizes[i] / (1024 * 1024) << " MB)\n";
                }
            }
        } else if (choice == "2") {
            std::cout << "Enter full file path: ";
            std::string file_path;
            std::getline(std::cin, file_path);
            bool found = false;
            fs::path query_path = file_path;
            // Searches for the file in the indexed list (csv file)
            for (size_t i = 0; i < files.size(); ++i) {
                if (files[i] == query_path) {
                    // Prints out the hash value for the file
                    std::cout << "Checksum for " << file_path << " is: " << hashes[i] << "\n";
                    found = true;
                    break;
                }
            }
        } else {
            break;
        }
    }
    return 0;
}