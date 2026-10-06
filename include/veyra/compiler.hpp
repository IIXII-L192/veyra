#pragma once
#include <string>
#include <vector>

namespace veyra {

struct CompileOptions {
    std::string source_file;
    std::string output_binary;
    bool run_after_build = false;
    bool emit_cpp_only = false;
    bool optimize = false;
    std::vector<std::string> extra_args;
    std::vector<std::string> user_program_args;
};

class Compiler {
public:
    Compiler();
    int run(const CompileOptions& options);
    std::string transpile_file(const std::string& filepath);

private:
    std::string find_host_cxx_compiler();
    std::string get_cache_directory();
    bool write_embedded_prelude(const std::string& dest_path);
};

// Returns the full embedded C++20 prelude source code
const char* get_embedded_prelude_source();

} // namespace veyra
