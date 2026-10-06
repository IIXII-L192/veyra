#include "veyra/compiler.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

void print_banner() {
    std::cout << "\033[1;36m"
              << " __      __                          \n"
              << " \\ \\    / /__ _   _ _ __ __ _       \n"
              << "  \\ \\  / / _ \\ | | | '__/ _` |      \n"
              << "   \\ \\/ /  __/ |_| | | | (_| |      \n"
              << "    \\__/ \\___|\\__, |_|  \\__,_|      \n"
              << "               |___/   v0.1.0-alpha \n"
              << "\033[0m"
              << " Simple as Python, Powerful as C++\n\n";
}

void print_help() {
    print_banner();
    std::cout << "\033[1mUSAGE:\033[0m\n"
              << "  veyra <file.vey> [args...]       Run a Veyra script directly in 1 command\n"
              << "  veyra run <file.vey> [args...]   Run a Veyra script directly\n"
              << "  veyra build <file.vey> [-o out]  Compile to a standalone native binary\n"
              << "  veyra emit <file.vey>            Inspect generated C++20 code\n"
              << "  veyra new <project_name>         Create a starter Veyra project\n"
              << "  veyra --version                  Display version info\n"
              << "  veyra --help                     Display this help message\n\n"
              << "\033[1mEXAMPLES:\033[0m\n"
              << "  veyra app.vey\n"
              << "  veyra build game.vey -o mygame -O3\n"
              << "  veyra emit main.vey\n";
}

void create_new_project(const std::string& name) {
    if (fs::exists(name)) {
        std::cerr << "\033[1;31mError:\033[0m Directory '" << name << "' already exists!\n";
        return;
    }
    fs::create_directories(name);
    std::string main_file = name + "/main.vey";
    std::ofstream out(main_file);
    out << "# Welcome to Veyra!\n"
        << "# Run with: veyra main.vey\n\n"
        << "fn main() {\n"
        << "    let message = \"Hello from Veyra!\"\n"
        << "    let numbers = [10, 20, 30, 40]\n\n"
        << "    println(\"{message}\")\n"
        << "    println(\"List: {numbers}\")\n\n"
        << "    for i, val in enumerate(numbers) {\n"
        << "        println(\"Item {i + 1}: {val}\")\n"
        << "    }\n"
        << "}\n";
    out.close();

    std::cout << "\033[1;32m✓ Created project:\033[0m " << name << "\n"
              << "To run:\n"
              << "  cd " << name << "\n"
              << "  veyra main.vey\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    std::string arg1 = argv[1];

    if (arg1 == "--help" || arg1 == "-h" || arg1 == "help") {
        print_help();
        return 0;
    }

    if (arg1 == "--version" || arg1 == "-v" || arg1 == "version") {
        std::cout << "Veyra Language Compiler v0.1.0 (Target: C++20)\n";
        return 0;
    }

    if (arg1 == "new") {
        if (argc < 3) {
            std::cerr << "\033[1;31mError:\033[0m Expected project name: veyra new <name>\n";
            return 1;
        }
        create_new_project(argv[2]);
        return 0;
    }

    veyra::Compiler compiler;
    veyra::CompileOptions opts;

    if (arg1 == "run") {
        if (argc < 3) {
            std::cerr << "\033[1;31mError:\033[0m Expected source file: veyra run <file.vey>\n";
            return 1;
        }
        opts.source_file = argv[2];
        opts.run_after_build = true;
        for (int i = 3; i < argc; ++i) {
            opts.user_program_args.push_back(argv[i]);
        }
    } else if (arg1 == "build") {
        if (argc < 3) {
            std::cerr << "\033[1;31mError:\033[0m Expected source file: veyra build <file.vey>\n";
            return 1;
        }
        opts.source_file = argv[2];
        opts.run_after_build = false;

        for (int i = 3; i < argc; ++i) {
            std::string flag = argv[i];
            if (flag == "-o" && i + 1 < argc) {
                opts.output_binary = argv[++i];
            } else if (flag == "-O3" || flag == "-O2" || flag == "-O0") {
                opts.optimize = (flag == "-O3");
            } else {
                opts.extra_args.push_back(flag);
            }
        }
    } else if (arg1 == "emit") {
        if (argc < 3) {
            std::cerr << "\033[1;31mError:\033[0m Expected source file: veyra emit <file.vey>\n";
            return 1;
        }
        opts.source_file = argv[2];
        opts.emit_cpp_only = true;
    } else {
        // Direct run: veyra file.vey [args...]
        opts.source_file = arg1;
        opts.run_after_build = true;
        for (int i = 2; i < argc; ++i) {
            opts.user_program_args.push_back(argv[i]);
        }
    }

    return compiler.run(opts);
}
