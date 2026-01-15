#include <algorithm>
#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;


// -----------------------------
// Small helpers
// -----------------------------
static std::string jsonEscape(const std::string& s) {
    // Minimal JSON string escape (good enough for file paths/names)
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c; break;
        }
    }
    return out;
}

static std::string permsToString(fs::perms p) {
    // POSIX-like rwx string, best-effort cross-platform
    auto bit = [&](fs::perms b) { return (p & b) != fs::perms::none; };

    std::string s;
    s += bit(fs::perms::owner_read)  ? 'r' : '-';
    s += bit(fs::perms::owner_write) ? 'w' : '-';
    s += bit(fs::perms::owner_exec)  ? 'x' : '-';

    s += bit(fs::perms::group_read)  ? 'r' : '-';
    s += bit(fs::perms::group_write) ? 'w' : '-';
    s += bit(fs::perms::group_exec)  ? 'x' : '-';

    s += bit(fs::perms::others_read)  ? 'r' : '-';
    s += bit(fs::perms::others_write) ? 'w' : '-';
    s += bit(fs::perms::others_exec)  ? 'x' : '-';
    return s;
}

// Convert filesystem::file_time_type to seconds since epoch (best-effort)
static long long fileTimeToEpochSeconds(fs::file_time_type ft) {
    // This conversion is a common approach for C++17; it’s “best-effort”
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(
        ft - fs::file_time_type::clock::now() + system_clock::now()
    );
    return duration_cast<seconds>(sctp.time_since_epoch()).count();
}

// -----------------------------
// Indexing work (the "CPU burst")
// -----------------------------
static std::string scanOnePathJson(const fs::path& p) {
    // Build ONE JSON object (as a string) representing the file record.
    // Keep it simple: path, name, size, last_write_time, type flags, permissions, errors.
    std::string pathStr = p.string();
    std::string nameStr = p.filename().string();

    bool is_file = false;
    bool is_dir = false;
    bool is_symlink = false;
    unsigned long long size_bytes = 0;
    long long mtime_epoch = 0;
    std::string perms_rwx = "---------";
    std::string error;

    try {
        // status() follows symlinks; symlink_status() does not
        fs::file_status st = fs::symlink_status(p);

        is_symlink = fs::is_symlink(st);
        is_file = fs::is_regular_file(st);
        is_dir = fs::is_directory(st);

        // Size only valid for regular files
        if (is_file) {
            size_bytes = fs::file_size(p);
        }

        // Last write time
        mtime_epoch = fileTimeToEpochSeconds(fs::last_write_time(p));

        // Permissions
        perms_rwx = permsToString(st.permissions());
    }
    catch (const fs::filesystem_error& e) {
        error = e.what();
    }
    catch (const std::exception& e) {
        error = e.what();
    }

    // Note: Full ACLs on Windows (SIDs/ACEs) require platform APIs / extra libs.
    // This template records basic permission bits only.

    std::string json = "{";
    json += "\"path\":\"" + jsonEscape(pathStr) + "\",";
    json += "\"name\":\"" + jsonEscape(nameStr) + "\",";
    json += "\"is_file\":" + std::string(is_file ? "true" : "false") + ",";
    json += "\"is_dir\":" + std::string(is_dir ? "true" : "false") + ",";
    json += "\"is_symlink\":" + std::string(is_symlink ? "true" : "false") + ",";
    json += "\"size_bytes\":" + std::to_string(size_bytes) + ",";
    json += "\"mtime_epoch\":" + std::to_string(mtime_epoch) + ",";
    json += "\"perms_rwx\":\"" + perms_rwx + "\"";

    if (!error.empty()) {
        json += ",\"error\":\"" + jsonEscape(error) + "\"";
    }

    json += "}";
    return json;
}




// -----------------------------
// Indexing loop (no scheduler)
// -----------------------------
// To be implemented: direct file processing without job queue or scheduling.

int main() {
    try {
        fs::path root = R"(C:\temp)";              // change me
        fs::path out  = "index_results.jsonl";     // output file
        runIndexer(root, out);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
