#include "veyra/lexer.hpp"
#include <cctype>
#include <unordered_map>
#include <stdexcept>

namespace veyra {

static const std::unordered_map<std::string, TokenType> kKeywords = {
    {"fn", TokenType::KW_FN},
    {"def", TokenType::KW_FN},
    {"let", TokenType::KW_LET},
    {"mut", TokenType::KW_MUT},
    {"struct", TokenType::KW_STRUCT},
    {"class", TokenType::KW_CLASS},
    {"enum", TokenType::KW_ENUM},
    {"if", TokenType::KW_IF},
    {"else", TokenType::KW_ELSE},
    {"elif", TokenType::KW_ELIF},
    {"while", TokenType::KW_WHILE},
    {"for", TokenType::KW_FOR},
    {"in", TokenType::KW_IN},
    {"return", TokenType::KW_RETURN},
    {"break", TokenType::KW_BREAK},
    {"continue", TokenType::KW_CONTINUE},
    {"true", TokenType::KW_TRUE},
    {"false", TokenType::KW_FALSE},
    {"null", TokenType::KW_NULL},
    {"import", TokenType::KW_IMPORT},
    {"cpp", TokenType::KW_CPP},
    {"cinclude", TokenType::KW_CINCLUDE},
    {"operator", TokenType::KW_OPERATOR},
    {"override", TokenType::KW_OVERRIDE},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
};

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::tokenize() {
    tokens_.clear();
    while (!is_at_end()) {
        skip_whitespace_and_comments();
        if (is_at_end()) break;
        start_ = current_;
        token_start_col_ = col_;
        scan_token();
    }
    tokens_.push_back({TokenType::END_OF_FILE, "", line_, col_});
    return tokens_;
}

char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[current_];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

char Lexer::advance() {
    char c = source_[current_++];
    col_++;
    return c;
}

bool Lexer::is_at_end() const {
    return current_ >= source_.size();
}

bool Lexer::match(char expected) {
    if (is_at_end()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    col_++;
    return true;
}

void Lexer::skip_whitespace_and_comments() {
    while (!is_at_end()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            line_++;
            col_ = 1;
            advance();
        } else if (c == '#') {
            // Python style comment
            while (!is_at_end() && peek() != '\n') {
                advance();
            }
        } else if (c == '/' && peek_next() == '/') {
            // Single line C++ comment
            while (!is_at_end() && peek() != '\n') {
                advance();
            }
        } else if (c == '/' && peek_next() == '*') {
            // Multi line comment
            advance(); // '/'
            advance(); // '*'
            while (!is_at_end() && !(peek() == '*' && peek_next() == '/')) {
                if (peek() == '\n') {
                    line_++;
                    col_ = 1;
                }
                advance();
            }
            if (!is_at_end()) {
                advance(); // '*'
                advance(); // '/'
            }
        } else {
            break;
        }
    }
}

void Lexer::scan_token() {
    char c = advance();

    // Check single character tokens and multi-character operators
    switch (c) {
        case '(': tokens_.push_back({TokenType::LPAREN, "(", line_, token_start_col_}); break;
        case ')': tokens_.push_back({TokenType::RPAREN, ")", line_, token_start_col_}); break;
        case '{': tokens_.push_back({TokenType::LBRACE, "{", line_, token_start_col_}); break;
        case '}': tokens_.push_back({TokenType::RBRACE, "}", line_, token_start_col_}); break;
        case '[': tokens_.push_back({TokenType::LBRACKET, "[", line_, token_start_col_}); break;
        case ']': tokens_.push_back({TokenType::RBRACKET, "]", line_, token_start_col_}); break;
        case ',': tokens_.push_back({TokenType::COMMA, ",", line_, token_start_col_}); break;
        case ':': tokens_.push_back({TokenType::COLON, ":", line_, token_start_col_}); break;
        case ';': tokens_.push_back({TokenType::SEMICOLON, ";", line_, token_start_col_}); break;
        case '?': tokens_.push_back({TokenType::QUESTION, "?", line_, token_start_col_}); break;

        case '.':
            if (match('.')) {
                tokens_.push_back({TokenType::DOT_DOT, "..", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::DOT, ".", line_, token_start_col_});
            }
            break;

        case '+':
            if (match('=')) {
                tokens_.push_back({TokenType::PLUS_ASSIGN, "+=", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::PLUS, "+", line_, token_start_col_});
            }
            break;

        case '-':
            if (match('>')) {
                tokens_.push_back({TokenType::ARROW, "->", line_, token_start_col_});
            } else if (match('=')) {
                tokens_.push_back({TokenType::MINUS_ASSIGN, "-=", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::MINUS, "-", line_, token_start_col_});
            }
            break;

        case '*':
            if (match('=')) {
                tokens_.push_back({TokenType::STAR_ASSIGN, "*=", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::STAR, "*", line_, token_start_col_});
            }
            break;

        case '/':
            if (match('=')) {
                tokens_.push_back({TokenType::SLASH_ASSIGN, "/=", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::SLASH, "/", line_, token_start_col_});
            }
            break;

        case '%':
            if (match('=')) {
                tokens_.push_back({TokenType::PERCENT_ASSIGN, "%=", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::PERCENT, "%", line_, token_start_col_});
            }
            break;

        case '=':
            if (match('=')) {
                tokens_.push_back({TokenType::EQ, "==", line_, token_start_col_});
            } else if (match('>')) {
                tokens_.push_back({TokenType::FAT_ARROW, "=>", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::ASSIGN, "=", line_, token_start_col_});
            }
            break;

        case '!':
            if (match('=')) {
                tokens_.push_back({TokenType::NEQ, "!=", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::NOT, "!", line_, token_start_col_});
            }
            break;

        case '<':
            if (match('=')) {
                tokens_.push_back({TokenType::LTE, "<=", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::LT, "<", line_, token_start_col_});
            }
            break;

        case '>':
            if (match('=')) {
                tokens_.push_back({TokenType::GTE, ">=", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::GT, ">", line_, token_start_col_});
            }
            break;

        case '&':
            if (match('&')) {
                tokens_.push_back({TokenType::AND, "&&", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::AMPERSAND, "&", line_, token_start_col_});
            }
            break;

        case '~':
            tokens_.push_back({TokenType::TILDE, "~", line_, token_start_col_});
            break;

        case '|':
            if (match('|')) {
                tokens_.push_back({TokenType::OR, "||", line_, token_start_col_});
            } else {
                tokens_.push_back({TokenType::UNKNOWN, "|", line_, token_start_col_});
            }
            break;

        case '"':
        case '\'':
            current_--; // Rewind quote
            col_--;
            scan_string();
            break;

        default:
            if (std::isalpha(c) || c == '_') {
                current_--; // Rewind to start of identifier
                col_--;
                scan_identifier_or_keyword();
            } else if (std::isdigit(c)) {
                current_--; // Rewind to start of number
                col_--;
                scan_number();
            } else {
                tokens_.push_back({TokenType::UNKNOWN, std::string(1, c), line_, token_start_col_});
            }
            break;
    }
}

void Lexer::scan_identifier_or_keyword() {
    start_ = current_;
    token_start_col_ = col_;
    while (!is_at_end() && (std::isalnum(peek()) || peek() == '_')) {
        advance();
    }
    std::string text = source_.substr(start_, current_ - start_);
    auto it = kKeywords.find(text);
    if (it != kKeywords.end()) {
        tokens_.push_back({it->second, text, line_, token_start_col_});
        // Check if keyword is `cpp` followed by `{`
        if (it->second == TokenType::KW_CPP) {
            skip_whitespace_and_comments();
            if (peek() == '{') {
                scan_raw_cpp();
            }
        }
    } else {
        tokens_.push_back({TokenType::IDENTIFIER, text, line_, token_start_col_});
    }
}

void Lexer::scan_number() {
    start_ = current_;
    token_start_col_ = col_;
    bool is_float = false;

    // Hex / Bin prefixes
    if (peek() == '0' && (peek_next() == 'x' || peek_next() == 'X' || peek_next() == 'b' || peek_next() == 'B')) {
        advance(); // 0
        advance(); // x/b
        while (!is_at_end() && (std::isxdigit(peek()) || peek() == '_')) {
            advance();
        }
        tokens_.push_back({TokenType::INT_LITERAL, source_.substr(start_, current_ - start_), line_, token_start_col_});
        return;
    }

    while (!is_at_end() && (std::isdigit(peek()) || peek() == '_')) {
        advance();
    }

    if (peek() == '.' && std::isdigit(peek_next())) {
        is_float = true;
        advance(); // '.'
        while (!is_at_end() && (std::isdigit(peek()) || peek() == '_')) {
            advance();
        }
    }

    if (peek() == 'f' || peek() == 'F') {
        is_float = true;
        advance();
    }

    std::string num_str = source_.substr(start_, current_ - start_);
    tokens_.push_back({is_float ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL, num_str, line_, token_start_col_});
}

void Lexer::scan_string() {
    start_ = current_;
    token_start_col_ = col_;
    char quote_char = advance(); // " or '
    std::string val;

    while (!is_at_end() && peek() != quote_char) {
        if (peek() == '\n') {
            line_++;
            col_ = 1;
        }
        if (peek() == '\\') {
            advance(); // backslash
            if (!is_at_end()) {
                char esc = advance();
                switch (esc) {
                    case 'n': val += '\n'; break;
                    case 't': val += '\t'; break;
                    case 'r': val += '\r'; break;
                    case '\\': val += '\\'; break;
                    case '"': val += '"'; break;
                    case '\'': val += '\''; break;
                    default: val += esc; break;
                }
            }
        } else {
            val += advance();
        }
    }

    if (!is_at_end()) {
        advance(); // closing quote
    }

    tokens_.push_back({TokenType::STRING_LITERAL, val, line_, token_start_col_});
}

void Lexer::scan_raw_cpp() {
    token_start_col_ = col_;
    advance(); // '{'
    int brace_depth = 1;
    size_t raw_start = current_;

    while (!is_at_end() && brace_depth > 0) {
        if (peek() == '{') brace_depth++;
        else if (peek() == '}') {
            brace_depth--;
            if (brace_depth == 0) break;
        } else if (peek() == '\n') {
            line_++;
            col_ = 1;
        }
        advance();
    }

    std::string cpp_code = source_.substr(raw_start, current_ - raw_start);
    if (!is_at_end() && peek() == '}') advance();

    tokens_.push_back({TokenType::RAW_CPP_BLOCK, cpp_code, line_, token_start_col_});
}

} // namespace veyra
