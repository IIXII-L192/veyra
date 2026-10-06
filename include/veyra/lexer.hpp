#pragma once
#include "veyra/ast.hpp"
#include <string>
#include <vector>

namespace veyra {

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    char peek() const;
    char peek_next() const;
    char advance();
    bool is_at_end() const;
    bool match(char expected);

    void scan_token();
    void scan_identifier_or_keyword();
    void scan_number();
    void scan_string();
    void scan_raw_cpp();
    void skip_whitespace_and_comments();

    std::string source_;
    size_t start_ = 0;
    size_t current_ = 0;
    size_t line_ = 1;
    size_t col_ = 1;
    size_t token_start_col_ = 1;

    std::vector<Token> tokens_;
};

} // namespace veyra
