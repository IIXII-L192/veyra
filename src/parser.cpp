#include "veyra/parser.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace veyra {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::END_OF_FILE;
}

const Token& Parser::advance() {
    if (!is_at_end()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& error_message) {
    if (check(type)) return advance();
    throw std::runtime_error("Parse error at line " + std::to_string(peek().line) +
                             ", col " + std::to_string(peek().column) + ": " + error_message +
                             " (found '" + peek().text + "')");
}

std::shared_ptr<ProgramNode> Parser::parse_program() {
    auto prog = std::make_shared<ProgramNode>();
    while (!is_at_end()) {
        // Skip stray semicolons
        if (match(TokenType::SEMICOLON)) continue;
        prog->statements.push_back(statement());
    }
    return prog;
}

StmtPtr Parser::statement() {
    if (match(TokenType::KW_LET) || match(TokenType::KW_MUT)) {
        return var_decl_stmt();
    }
    if (match(TokenType::KW_FN)) {
        return fn_decl_stmt();
    }
    if (match(TokenType::KW_STRUCT) || match(TokenType::KW_CLASS)) {
        return struct_decl_stmt();
    }
    if (match(TokenType::KW_ENUM)) {
        return enum_decl_stmt();
    }
    if (match(TokenType::KW_IMPORT)) {
        return import_stmt();
    }
    if (match(TokenType::KW_IF)) {
        return if_stmt();
    }
    if (match(TokenType::KW_WHILE)) {
        return while_stmt();
    }
    if (match(TokenType::KW_FOR)) {
        return for_stmt();
    }
    if (match(TokenType::KW_RETURN)) {
        return return_stmt();
    }
    if (match(TokenType::KW_BREAK)) {
        auto b = std::make_shared<BreakStmt>();
        b->line = previous().line;
        match(TokenType::SEMICOLON);
        return b;
    }
    if (match(TokenType::KW_CONTINUE)) {
        auto c = std::make_shared<ContinueStmt>();
        c->line = previous().line;
        match(TokenType::SEMICOLON);
        return c;
    }
    if (match(TokenType::KW_CINCLUDE)) {
        auto inc = std::make_shared<CIncludeStmt>();
        inc->line = previous().line;
        if (check(TokenType::STRING_LITERAL) || check(TokenType::LT) || check(TokenType::IDENTIFIER)) {
            std::string header_str;
            if (match(TokenType::STRING_LITERAL)) {
                header_str = "\"" + previous().text + "\"";
            } else if (match(TokenType::LT)) {
                header_str = "<";
                while (!check(TokenType::GT) && !is_at_end()) {
                    header_str += advance().text;
                }
                consume(TokenType::GT, "Expected '>' after include path");
                header_str += ">";
            }
            inc->header = header_str;
        }
        match(TokenType::SEMICOLON);
        return inc;
    }
    if (match(TokenType::RAW_CPP_BLOCK)) {
        auto raw = std::make_shared<RawCppStmt>();
        raw->line = previous().line;
        raw->code = previous().text;
        return raw;
    }
    if (check(TokenType::LBRACE)) {
        return block_stmt();
    }

    return expr_or_assignment_stmt();
}

StmtPtr Parser::var_decl_stmt() {
    bool is_mut = (previous().type == TokenType::KW_MUT);
    auto stmt = std::make_shared<VarDeclStmt>();
    stmt->line = previous().line;
    stmt->is_mut = is_mut;

    const auto& name_tok = consume(TokenType::IDENTIFIER, "Expected variable name after let/mut");
    stmt->name = name_tok.text;

    if (match(TokenType::COLON)) {
        stmt->type_annot = parse_type_annotation();
    }

    if (match(TokenType::ASSIGN)) {
        stmt->init_expr = expression();
    }

    match(TokenType::SEMICOLON);
    return stmt;
}

std::string Parser::parse_type_annotation() {
    std::string type_str = "";
    while (check(TokenType::STAR) || check(TokenType::AMPERSAND) || check(TokenType::UNKNOWN)) {
        type_str += advance().text;
    }
    if (check(TokenType::IDENTIFIER) || check(TokenType::KW_LET) || check(TokenType::KW_MUT)) {
        type_str += advance().text;
    }
    if (match(TokenType::LBRACKET)) {
        type_str += "[";
        int depth = 1;
        while (!is_at_end() && depth > 0) {
            if (check(TokenType::LBRACKET)) depth++;
            else if (check(TokenType::RBRACKET)) {
                depth--;
                if (depth == 0) {
                    advance();
                    type_str += "]";
                    break;
                }
            }
            type_str += advance().text;
        }
    }
    while (check(TokenType::STAR) || check(TokenType::AMPERSAND) || check(TokenType::UNKNOWN)) {
        type_str += advance().text;
    }
    return type_str;
}

StmtPtr Parser::fn_decl_stmt(bool is_method) {
    auto stmt = std::make_shared<FnDeclStmt>();
    stmt->line = previous().line;
    stmt->is_method = is_method;

    if (match(TokenType::KW_OPERATOR)) {
        stmt->is_operator = true;
        // Consume operator token
        Token op_tok = advance();
        stmt->op_symbol = op_tok.text;
        if (op_tok.type == TokenType::LBRACKET && check(TokenType::RBRACKET)) {
            advance();
            stmt->op_symbol = "[]";
        }
        stmt->name = "operator" + stmt->op_symbol;
    } else if (match(TokenType::TILDE)) {
        stmt->is_destructor = true;
        stmt->name = "~" + consume(TokenType::IDENTIFIER, "Expected destructor name").text;
    } else {
        const auto& name_tok = consume(TokenType::IDENTIFIER, "Expected function name");
        stmt->name = name_tok.text;
        if (stmt->name == "drop") {
            stmt->is_destructor = true;
        }
    }

    // Optional template/generic params: fn foo[T, U](...)
    if (match(TokenType::LBRACKET)) {
        while (!check(TokenType::RBRACKET) && !is_at_end()) {
            stmt->generic_params.push_back(consume(TokenType::IDENTIFIER, "Expected generic parameter name").text);
            match(TokenType::COMMA);
        }
        consume(TokenType::RBRACKET, "Expected ']' after generic parameters");
    }

    consume(TokenType::LPAREN, "Expected '(' after function name");

    while (!check(TokenType::RPAREN) && !is_at_end()) {
        ParamDecl param;
        if (match(TokenType::KW_MUT)) {
            param.is_mut = true;
        }
        param.name = consume(TokenType::IDENTIFIER, "Expected parameter name").text;
        if (match(TokenType::COLON)) {
            param.type_annot = parse_type_annotation();
        }
        if (match(TokenType::ASSIGN)) {
            param.default_val = expression();
        }
        stmt->params.push_back(param);
        match(TokenType::COMMA);
    }
    consume(TokenType::RPAREN, "Expected ')' after parameters");

    if (match(TokenType::KW_OVERRIDE)) {
        stmt->is_override = true;
    }

    if (match(TokenType::ARROW)) {
        stmt->return_type = parse_type_annotation();
    }

    if (match(TokenType::KW_OVERRIDE)) {
        stmt->is_override = true;
    }

    if (match(TokenType::FAT_ARROW)) {
        stmt->arrow_body = expression();
        match(TokenType::SEMICOLON);
    } else {
        stmt->body = block_stmt();
    }

    return stmt;
}

StmtPtr Parser::struct_decl_stmt() {
    auto stmt = std::make_shared<StructDeclStmt>();
    stmt->line = previous().line;
    stmt->is_class = (previous().type == TokenType::KW_CLASS);

    const auto& name_tok = consume(TokenType::IDENTIFIER, "Expected struct/class name");
    stmt->name = name_tok.text;

    if (match(TokenType::LBRACKET)) {
        while (!check(TokenType::RBRACKET) && !is_at_end()) {
            stmt->generic_params.push_back(consume(TokenType::IDENTIFIER, "Expected generic type").text);
            match(TokenType::COMMA);
        }
        consume(TokenType::RBRACKET, "Expected ']' after generic parameters");
    }

    // Check inheritance: struct Dog : Animal
    if (match(TokenType::COLON)) {
        stmt->base_class = consume(TokenType::IDENTIFIER, "Expected base class name after ':'").text;
    }

    consume(TokenType::LBRACE, "Expected '{' to start struct body");

    while (!check(TokenType::RBRACE) && !is_at_end()) {
        if (match(TokenType::SEMICOLON)) continue;

        if (match(TokenType::KW_FN) || match(TokenType::KW_DEF)) {
            auto method = std::dynamic_pointer_cast<FnDeclStmt>(fn_decl_stmt(true));
            StructMember member;
            member.name = method->name;
            member.is_method = true;
            member.method = method;
            stmt->members.push_back(member);
        } else {
            StructMember member;
            member.name = consume(TokenType::IDENTIFIER, "Expected struct field name").text;
            if (match(TokenType::COLON)) {
                member.type_annot = parse_type_annotation();
            }
            if (match(TokenType::ASSIGN)) {
                member.default_val = expression();
            }
            stmt->members.push_back(member);
            match(TokenType::SEMICOLON);
            match(TokenType::COMMA);
        }
    }

    consume(TokenType::RBRACE, "Expected '}' to close struct body");
    match(TokenType::SEMICOLON);
    return stmt;
}

StmtPtr Parser::enum_decl_stmt() {
    auto stmt = std::make_shared<EnumDeclStmt>();
    stmt->line = previous().line;

    const auto& name_tok = consume(TokenType::IDENTIFIER, "Expected enum name");
    stmt->name = name_tok.text;

    consume(TokenType::LBRACE, "Expected '{' to start enum body");

    while (!check(TokenType::RBRACE) && !is_at_end()) {
        if (match(TokenType::SEMICOLON) || match(TokenType::COMMA)) continue;

        EnumMember m;
        m.name = consume(TokenType::IDENTIFIER, "Expected enum member name").text;
        if (match(TokenType::ASSIGN)) {
            m.value = expression();
        }
        stmt->members.push_back(m);
        match(TokenType::COMMA);
        match(TokenType::SEMICOLON);
    }

    consume(TokenType::RBRACE, "Expected '}' to close enum body");
    match(TokenType::SEMICOLON);
    return stmt;
}

StmtPtr Parser::import_stmt() {
    auto stmt = std::make_shared<ImportStmt>();
    stmt->line = previous().line;

    std::string mod_name = "";
    if (check(TokenType::STRING_LITERAL)) {
        mod_name = advance().text;
    } else {
        mod_name = consume(TokenType::IDENTIFIER, "Expected module name after import").text;
        while (match(TokenType::DOT)) {
            mod_name += "/" + consume(TokenType::IDENTIFIER, "Expected submodule identifier").text;
        }
    }

    stmt->module_name = mod_name;
    match(TokenType::SEMICOLON);
    return stmt;
}

StmtPtr Parser::if_stmt() {
    auto stmt = std::make_shared<IfStmt>();
    stmt->line = previous().line;

    stmt->condition = expression();
    stmt->then_branch = block_stmt();

    if (match(TokenType::KW_ELSE)) {
        if (match(TokenType::KW_IF)) {
            stmt->else_branch = if_stmt();
        } else {
            stmt->else_branch = block_stmt();
        }
    }

    return stmt;
}

StmtPtr Parser::while_stmt() {
    auto stmt = std::make_shared<WhileStmt>();
    stmt->line = previous().line;

    stmt->condition = expression();
    stmt->body = block_stmt();

    return stmt;
}

StmtPtr Parser::for_stmt() {
    size_t line = previous().line;
    bool is_mut = false;
    if (match(TokenType::KW_MUT)) {
        is_mut = true;
    }

    std::string var1 = consume(TokenType::IDENTIFIER, "Expected loop variable name").text;
    std::string var2 = "";

    if (match(TokenType::COMMA)) {
        // e.g. for i, item in enumerate(...)
        var2 = consume(TokenType::IDENTIFIER, "Expected second loop variable name").text;
    }

    consume(TokenType::KW_IN, "Expected 'in' after for variables");

    ExprPtr iter_expr = expression();

    // Check if iterable is a range expression start..stop
    if (auto range = std::dynamic_pointer_cast<RangeExpr>(iter_expr)) {
        auto r_stmt = std::make_shared<ForRangeStmt>();
        r_stmt->line = line;
        r_stmt->var_name = var1;
        r_stmt->start = range->start;
        r_stmt->stop = range->stop;
        r_stmt->step = range->step;
        r_stmt->body = block_stmt();
        return r_stmt;
    }

    auto for_in = std::make_shared<ForInStmt>();
    for_in->line = line;
    for_in->is_mut = is_mut;
    if (!var2.empty()) {
        for_in->index_name = var1;
        for_in->var_name = var2;
    } else {
        for_in->var_name = var1;
    }
    for_in->iterable = iter_expr;
    for_in->body = block_stmt();
    return for_in;
}

StmtPtr Parser::return_stmt() {
    auto stmt = std::make_shared<ReturnStmt>();
    stmt->line = previous().line;

    if (!check(TokenType::SEMICOLON) && !check(TokenType::RBRACE) && !is_at_end()) {
        stmt->value = expression();
    }

    match(TokenType::SEMICOLON);
    return stmt;
}

StmtPtr Parser::block_stmt() {
    auto stmt = std::make_shared<BlockStmt>();
    stmt->line = peek().line;

    consume(TokenType::LBRACE, "Expected '{' to start block");

    while (!check(TokenType::RBRACE) && !is_at_end()) {
        if (match(TokenType::SEMICOLON)) continue;
        stmt->statements.push_back(statement());
    }

    consume(TokenType::RBRACE, "Expected '}' to end block");
    return stmt;
}

StmtPtr Parser::expr_or_assignment_stmt() {
    auto expr = expression();
    auto stmt = std::make_shared<ExprStmt>();
    stmt->line = expr->line;
    stmt->expr = expr;
    match(TokenType::SEMICOLON);
    return stmt;
}

ExprPtr Parser::expression() {
    return assignment_expr();
}

ExprPtr Parser::assignment_expr() {
    ExprPtr expr = ternary_expr();

    if (match({TokenType::ASSIGN, TokenType::PLUS_ASSIGN, TokenType::MINUS_ASSIGN,
               TokenType::STAR_ASSIGN, TokenType::SLASH_ASSIGN, TokenType::PERCENT_ASSIGN})) {
        TokenType op = previous().type;
        ExprPtr value = assignment_expr();

        auto bin = std::make_shared<BinaryExpr>();
        bin->line = previous().line;
        bin->left = expr;
        bin->op = op;
        bin->right = value;
        return bin;
    }

    return expr;
}

ExprPtr Parser::ternary_expr() {
    ExprPtr expr = logical_or_expr();

    if (match(TokenType::QUESTION)) {
        ExprPtr then_branch = expression();
        consume(TokenType::COLON, "Expected ':' in ternary conditional");
        ExprPtr else_branch = ternary_expr();

        auto tern = std::make_shared<TernaryExpr>();
        tern->line = previous().line;
        tern->condition = expr;
        tern->then_expr = then_branch;
        tern->else_expr = else_branch;
        return tern;
    }

    return expr;
}

ExprPtr Parser::logical_or_expr() {
    ExprPtr expr = logical_and_expr();

    while (match(TokenType::OR)) {
        TokenType op = previous().type;
        ExprPtr right = logical_and_expr();
        auto bin = std::make_shared<BinaryExpr>();
        bin->line = previous().line;
        bin->left = expr;
        bin->op = op;
        bin->right = right;
        expr = bin;
    }

    return expr;
}

ExprPtr Parser::logical_and_expr() {
    ExprPtr expr = equality_expr();

    while (match(TokenType::AND)) {
        TokenType op = previous().type;
        ExprPtr right = equality_expr();
        auto bin = std::make_shared<BinaryExpr>();
        bin->line = previous().line;
        bin->left = expr;
        bin->op = op;
        bin->right = right;
        expr = bin;
    }

    return expr;
}

ExprPtr Parser::equality_expr() {
    ExprPtr expr = comparison_expr();

    while (match({TokenType::EQ, TokenType::NEQ})) {
        TokenType op = previous().type;
        ExprPtr right = comparison_expr();
        auto bin = std::make_shared<BinaryExpr>();
        bin->line = previous().line;
        bin->left = expr;
        bin->op = op;
        bin->right = right;
        expr = bin;
    }

    return expr;
}

ExprPtr Parser::comparison_expr() {
    ExprPtr expr = range_expr();

    while (match({TokenType::LT, TokenType::GT, TokenType::LTE, TokenType::GTE})) {
        TokenType op = previous().type;
        ExprPtr right = range_expr();
        auto bin = std::make_shared<BinaryExpr>();
        bin->line = previous().line;
        bin->left = expr;
        bin->op = op;
        bin->right = right;
        expr = bin;
    }

    return expr;
}

ExprPtr Parser::range_expr() {
    ExprPtr expr = term_expr();

    if (match(TokenType::DOT_DOT)) {
        ExprPtr stop = term_expr();
        auto r = std::make_shared<RangeExpr>();
        r->line = previous().line;
        r->start = expr;
        r->stop = stop;
        return r;
    }

    return expr;
}

ExprPtr Parser::term_expr() {
    ExprPtr expr = factor_expr();

    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        if (peek().line > previous().line) break;
        advance();
        TokenType op = previous().type;
        ExprPtr right = factor_expr();
        auto bin = std::make_shared<BinaryExpr>();
        bin->line = previous().line;
        bin->left = expr;
        bin->op = op;
        bin->right = right;
        expr = bin;
    }

    return expr;
}

ExprPtr Parser::factor_expr() {
    ExprPtr expr = unary_expr();

    while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)) {
        if (peek().line > previous().line) break;
        advance();
        TokenType op = previous().type;
        ExprPtr right = unary_expr();
        auto bin = std::make_shared<BinaryExpr>();
        bin->line = previous().line;
        bin->left = expr;
        bin->op = op;
        bin->right = right;
        expr = bin;
    }

    return expr;
}

ExprPtr Parser::unary_expr() {
    if (match({TokenType::NOT, TokenType::MINUS, TokenType::PLUS, TokenType::AMPERSAND, TokenType::STAR, TokenType::TILDE})) {
        TokenType op = previous().type;
        ExprPtr operand = unary_expr();
        auto un = std::make_shared<UnaryExpr>();
        un->line = previous().line;
        un->op = op;
        un->operand = operand;
        return un;
    }

    return postfix_expr();
}

ExprPtr Parser::postfix_expr() {
    ExprPtr expr = primary_expr();

    while (true) {
        if (match(TokenType::LPAREN)) {
            // Function call
            auto call = std::make_shared<CallExpr>();
            call->line = previous().line;
            call->callee = expr;

            if (!check(TokenType::RPAREN)) {
                do {
                    call->args.push_back(expression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after function arguments");
            expr = call;
        } else if (match(TokenType::DOT)) {
            // Member access: expr.member
            auto member = std::make_shared<MemberAccessExpr>();
            member->line = previous().line;
            member->object = expr;
            member->member = consume(TokenType::IDENTIFIER, "Expected member name after '.'").text;
            expr = member;
        } else if (match(TokenType::LBRACKET)) {
            // Index access: expr[idx]
            auto idx = std::make_shared<IndexExpr>();
            idx->line = previous().line;
            idx->object = expr;
            idx->index = expression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            expr = idx;
        } else {
            break;
        }
    }

    return expr;
}

ExprPtr Parser::parse_string_literal(const Token& tok) {
    const std::string& raw = tok.text;
    if (raw.find('{') == std::string::npos) {
        auto lit = std::make_shared<LiteralExpr>();
        lit->line = tok.line;
        lit->lit_type = TokenType::STRING_LITERAL;
        lit->value = raw;
        return lit;
    }

    // String interpolation: "Hello {name}, 1+1={1+1}"
    auto interp = std::make_shared<StringInterpolationExpr>();
    interp->line = tok.line;

    size_t i = 0;
    while (i < raw.size()) {
        if (raw[i] == '{' && (i == 0 || raw[i - 1] != '\\')) {
            size_t start_expr = i + 1;
            int depth = 1;
            size_t j = start_expr;
            while (j < raw.size() && depth > 0) {
                if (raw[j] == '{') depth++;
                else if (raw[j] == '}') depth--;
                j++;
            }
            if (depth == 0) {
                std::string code_sub = raw.substr(start_expr, j - start_expr - 1);
                // Parse sub-expression
                Lexer sub_lexer(code_sub);
                auto sub_tokens = sub_lexer.tokenize();
                Parser sub_parser(sub_tokens);
                ExprPtr sub_expr = sub_parser.expression();

                StringInterpolationExpr::Part part;
                part.is_expr = true;
                part.expr = sub_expr;
                interp->parts.push_back(part);

                i = j;
                continue;
            }
        }

        // Text part
        size_t next_bracket = raw.find('{', i);
        if (next_bracket == std::string::npos) {
            StringInterpolationExpr::Part part;
            part.is_expr = false;
            part.raw_text = raw.substr(i);
            interp->parts.push_back(part);
            break;
        } else {
            StringInterpolationExpr::Part part;
            part.is_expr = false;
            part.raw_text = raw.substr(i, next_bracket - i);
            interp->parts.push_back(part);
            i = next_bracket;
        }
    }

    return interp;
}

ExprPtr Parser::primary_expr() {
    if (match(TokenType::INT_LITERAL) || match(TokenType::FLOAT_LITERAL) ||
        match(TokenType::KW_TRUE) || match(TokenType::KW_FALSE) || match(TokenType::KW_NULL)) {
        auto lit = std::make_shared<LiteralExpr>();
        lit->line = previous().line;
        lit->lit_type = previous().type;
        lit->value = previous().text;
        return lit;
    }

    if (match(TokenType::STRING_LITERAL)) {
        return parse_string_literal(previous());
    }

    if (match(TokenType::IDENTIFIER)) {
        auto id = std::make_shared<IdentifierExpr>();
        id->line = previous().line;
        id->name = previous().text;
        return id;
    }

    if (match(TokenType::LPAREN)) {
        ExprPtr expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after parenthesized expression");
        return expr;
    }

    if (match(TokenType::LBRACKET)) {
        // Vector literal [1, 2, 3]
        auto vec_lit = std::make_shared<VectorLiteralExpr>();
        vec_lit->line = previous().line;

        if (!check(TokenType::RBRACKET)) {
            do {
                vec_lit->elements.push_back(expression());
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RBRACKET, "Expected ']' after vector elements");
        return vec_lit;
    }

    throw std::runtime_error("Unexpected token '" + peek().text + "' at line " +
                             std::to_string(peek().line) + ", col " + std::to_string(peek().column));
}

} // namespace veyra
