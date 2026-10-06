#pragma once
#include "veyra/ast.hpp"
#include "veyra/lexer.hpp"
#include <vector>
#include <string>

namespace veyra {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::shared_ptr<ProgramNode> parse_program();

private:
    const Token& peek() const;
    const Token& previous() const;
    bool is_at_end() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    const Token& consume(TokenType type, const std::string& error_message);

    // Statements
    StmtPtr statement();
    StmtPtr var_decl_stmt();
    std::string parse_type_annotation();
    StmtPtr fn_decl_stmt(bool is_method = false);
    StmtPtr struct_decl_stmt();
    StmtPtr enum_decl_stmt();
    StmtPtr import_stmt();
    StmtPtr if_stmt();
    StmtPtr while_stmt();
    StmtPtr for_stmt();
    StmtPtr return_stmt();
    StmtPtr block_stmt();
    StmtPtr expr_or_assignment_stmt();

    // Expressions
    ExprPtr expression();
    ExprPtr assignment_expr();
    ExprPtr ternary_expr();
    ExprPtr logical_or_expr();
    ExprPtr logical_and_expr();
    ExprPtr equality_expr();
    ExprPtr comparison_expr();
    ExprPtr range_expr();
    ExprPtr term_expr();
    ExprPtr factor_expr();
    ExprPtr unary_expr();
    ExprPtr postfix_expr();
    ExprPtr primary_expr();

    ExprPtr parse_string_literal(const Token& tok);

    std::vector<Token> tokens_;
    size_t current_ = 0;
};

} // namespace veyra
