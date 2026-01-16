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
#include <openssl/sha.h>

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
// Calculating hash value of files
std::string sha256sum(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return "";
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    char buf[8192];
    while (f.good()) {
        f.read(buf, sizeof(buf));
        SHA256_Update(&ctx, buf, f.gcount());
    }
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &ctx);
    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}
// End of AI aided help

int main() {
    // Option to change hashing [Manual work]
    std::string hash_option;
    std::count << "Select hash algorithm: "
              << "\n1. sha-256 (default)"
              << "\n2. sha-1"
              << "\n3. md5"
              << "\nEnter choice (1-3): ";
    std::getline(std::cin, hash_option);
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

    // Scan for files
    std::cout << "\nScanning " << root << " for files...\n";
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (fs::is_regular_file(entry.path())) {
            files.push_back(entry.path());
        }
    }
    auto wall_end = std::chrono::steady_clock::now();
    auto cpu_end = std::clock();
    std::cout << "Found " << files.size() << " files in "
              << std::chrono::duration<double>(wall_end - wall_start).count() << " real seconds and "
              << double(cpu_end - cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Getting file sizes
    wall_start = std::chrono::steady_clock::now();
    cpu_start = std::clock();
    std::vector<uintmax_t> sizes(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        try {
            sizes[i] = fs::file_size(files[i]);
        } catch (...) { sizes[i] = 0; }
    }
    wall_end = std::chrono::steady_clock::now();
    cpu_end = std::clock();
    std::cout << "Got sizes for " << files.size() << " files in "
              << std::chrono::duration<double>(wall_end - wall_start).count() << " real seconds and "
              << double(cpu_end - cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Getting file mtimes
    wall_start = std::chrono::steady_clock::now();
    cpu_start = std::clock();
    std::vector<long long> mtimes(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        try {
            mtimes[i] = fileTimeToEpochSeconds(fs::last_write_time(files[i]));
        } catch (...) { mtimes[i] = 0; }
    }
    wall_end = std::chrono::steady_clock::now();
    cpu_end = std::clock();
    std::cout << "Got mtimes for " << files.size() << " files in "
              << std::chrono::duration<double>(wall_end - wall_start).count() << " real seconds and "
              << double(cpu_end - cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Getting file owners from files
    wall_start = std::chrono::steady_clock::now();
    cpu_start = std::clock();
    std::vector<std::string> owners(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        owners[i] = getOwner(files[i]);
    }
    wall_end = std::chrono::steady_clock::now();
    cpu_end = std::clock();
    std::cout << "Got owners for " << files.size() << " files in "
              << std::chrono::duration<double>(wall_end - wall_start).count() << " real seconds and "
              << double(cpu_end - cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Getting file hashes from the files
    wall_start = std::chrono::steady_clock::now();
    cpu_start = std::clock();
    std::vector<std::string> hashes(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        hashes[i] = sha256sum(files[i]);
    }
    wall_end = std::chrono::steady_clock::now();
    cpu_end = std::clock();
    std::cout << "Calculated hashes for " << files.size() << " files in "
              << std::chrono::duration<double>(wall_end - wall_start).count() << " real seconds and "
              << double(cpu_end - cpu_start) / CLOCKS_PER_SEC << " CPU seconds\n";

    // Write to CSV
    std::ofstream csv(csv_file);
    csv << "filename,size,mtime,owner,hash\n";
    for (size_t i = 0; i < files.size(); ++i) {
        csv << '"' << files[i].string() << '"' << ',' << sizes[i] << ',' << mtimes[i] << ',' << owners[i] << ',' << hashes[i] << "\n";
    }
    csv.close();
    std::cout << "\nSaved file index information to " << csv_file << " with " << files.size() << " entries\n";
    return 0;
    // End of AI help, rest of code is manually written
    
    // Creating interactive query menu for users:
