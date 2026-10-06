#pragma once
#include "veyra/ast.hpp"
#include <string>
#include <sstream>
#include <unordered_set>

namespace veyra {

class CodeGen {
public:
    CodeGen();
    std::string generate(const std::shared_ptr<ProgramNode>& program);

private:
    void gen_stmt(const StmtPtr& stmt);
    void gen_expr(const ExprPtr& expr);

    std::string translate_type(const std::string& vtype);
    void indent();

    std::stringstream out_;
    int indent_level_ = 0;
    std::unordered_set<std::string> known_enums_;
};

} // namespace veyra
