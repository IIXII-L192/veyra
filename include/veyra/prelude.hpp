#pragma once

// Veyra Standard Prelude - Modern C++20 Core
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <optional>
#include <memory>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>
#include <utility>
#include <cstdint>
#include <format>
#include <concepts>
#include <ranges>

namespace veyra_core {

// Basic Type Aliases
using string = std::string;
using string_view = std::string_view;

template <typename T>
using vec = std::vector<T>;

template <typename K, typename V>
using map = std::unordered_map<K, V>;

template <typename T>
using set = std::unordered_set<T>;

template <typename T>
using opt = std::optional<T>;

template <typename A, typename B>
using pair = std::pair<A, B>;

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using byte = uint8_t;

using f32 = float;
using f64 = double;

// Custom Output Stream operator for vectors
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        os << v[i];
        if (i + 1 < v.size()) os << ", ";
    }
    os << "]";
    return os;
}

// Custom Output Stream operator for maps
template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<K, V>& m) {
    os << "{";
    size_t i = 0;
    for (const auto& [k, val] : m) {
        os << k << ": " << val;
        if (++i < m.size()) os << ", ";
    }
    os << "}";
    return os;
}

// Custom Output Stream operator for pairs
template <typename A, typename B>
std::ostream& operator<<(std::ostream& os, const std::pair<A, B>& p) {
    os << "(" << p.first << ", " << p.second << ")";
    return os;
}

// Printing Functions
template <typename... Args>
void print(Args&&... args) {
    auto print_one = [](const auto& arg) {
        std::cout << arg;
    };
    int dummy[] = {0, ((std::cout << (dummy[0]++ ? " " : ""), print_one(args)), 0)...};
    (void)dummy;
}

template <typename... Args>
void println(Args&&... args) {
    if constexpr (sizeof...(args) > 0) {
        print(std::forward<Args>(args)...);
    }
    std::cout << "\n";
}

// String & Collection Utilities
template <typename Container>
size_t len(const Container& c) {
    return c.size();
}

template <typename T>
size_t len(const std::vector<T>& c) {
    return c.size();
}

inline size_t len(const std::string& s) {
    return s.size();
}

template <typename T>
void push(std::vector<T>& v, const T& val) {
    v.push_back(val);
}

template <typename T>
void push(std::vector<T>& v, T&& val) {
    v.push_back(std::move(val));
}

template <typename T>
T pop(std::vector<T>& v) {
    if (v.empty()) throw std::runtime_error("Cannot pop from empty vector");
    T val = std::move(v.back());
    v.pop_back();
    return val;
}

template <typename T>
void sort(std::vector<T>& v) {
    std::sort(v.begin(), v.end());
}

template <typename T, typename Comp>
void sort(std::vector<T>& v, Comp comp) {
    std::sort(v.begin(), v.end(), comp);
}

template <typename T>
void reverse(std::vector<T>& v) {
    std::reverse(v.begin(), v.end());
}

template <typename T>
bool contains(const std::vector<T>& v, const T& item) {
    return std::find(v.begin(), v.end(), item) != v.end();
}

template <typename K, typename V>
bool contains(const std::unordered_map<K, V>& m, const K& key) {
    return m.find(key) != m.end();
}

inline bool contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

// Range Generation
inline std::vector<int64_t> range(int64_t stop) {
    std::vector<int64_t> res;
    if (stop > 0) {
        res.reserve(stop);
        for (int64_t i = 0; i < stop; ++i) res.push_back(i);
    }
    return res;
}

inline std::vector<int64_t> range(int64_t start, int64_t stop, int64_t step = 1) {
    std::vector<int64_t> res;
    if (step > 0) {
        for (int64_t i = start; i < stop; i += step) res.push_back(i);
    } else if (step < 0) {
        for (int64_t i = start; i > stop; i += step) res.push_back(i);
    }
    return res;
}

// Enumerate Helper
template <typename Container>
auto enumerate(const Container& c) {
    using ValueType = typename Container::value_type;
    std::vector<std::pair<size_t, ValueType>> result;
    result.reserve(c.size());
    size_t idx = 0;
    for (const auto& item : c) {
        result.push_back({idx++, item});
    }
    return result;
}

// I/O & File Management
inline std::string input(const std::string& prompt = "") {
    if (!prompt.empty()) {
        std::cout << prompt;
        std::cout.flush();
    }
    std::string line;
    std::getline(std::cin, line);
    return line;
}

inline int64_t input_int(const std::string& prompt = "") {
    std::string s = input(prompt);
    return std::stoll(s);
}

inline double input_float(const std::string& prompt = "") {
    std::string s = input(prompt);
    return std::stod(s);
}

inline std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline std::vector<std::string> read_lines(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + path);
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

inline bool write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    return true;
}

inline bool append_file(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) return false;
    file << content;
    return true;
}

// Conversion & Math Helpers
template <typename T>
std::string str(const T& val) {
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        return val;
    } else {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
}

inline int64_t to_int(const std::string& s) { return std::stoll(s); }
inline double to_float(const std::string& s) { return std::stod(s); }

inline int64_t rand_int(int64_t min_val, int64_t max_val) {
    static std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int64_t> dist(min_val, max_val);
    return dist(rng);
}

inline double rand_float(double min_val = 0.0, double max_val = 1.0) {
    static std::mt19937_64 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(min_val, max_val);
    return dist(rng);
}

// Time Helpers
inline int64_t time_now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

inline double time_now_sec() {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

inline void sleep_ms(int64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Math Forwarding
using std::abs;
using std::min;
using std::max;
using std::clamp;
using std::sqrt;
using std::pow;
using std::sin;
using std::cos;
using std::tan;
using std::floor;
using std::ceil;
using std::round;

} // namespace veyra_core

// Bring entire veyra_core into root scope for .vey compilation
using namespace veyra_core;
