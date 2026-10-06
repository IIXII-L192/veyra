#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace veyra {

enum class TokenType {
    // Keywords
    KW_FN,
    KW_DEF,
    KW_LET,
    KW_MUT,
    KW_STRUCT,
    KW_CLASS,
    KW_ENUM,
    KW_IF,
    KW_ELSE,
    KW_ELIF,
    KW_WHILE,
    KW_FOR,
    KW_IN,
    KW_RETURN,
    KW_BREAK,
    KW_CONTINUE,
    KW_TRUE,
    KW_FALSE,
    KW_NULL,
    KW_IMPORT,
    KW_CPP,
    KW_CINCLUDE,
    KW_OPERATOR,
    KW_OVERRIDE,

    // Identifiers & Literals
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    RAW_CPP_BLOCK,

    // Operators & Punctuation
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PERCENT,        // %
    ASSIGN,         // =
    PLUS_ASSIGN,    // +=
    MINUS_ASSIGN,   // -=
    STAR_ASSIGN,    // *=
    SLASH_ASSIGN,   // /=
    PERCENT_ASSIGN, // %=

    EQ,             // ==
    NEQ,            // !=
    LT,             // <
    GT,             // >
    LTE,            // <=
    GTE,            // >=

    AND,            // && or and
    OR,             // || or or
    NOT,            // ! or not
    AMPERSAND,      // & (address-of / ref)
    TILDE,          // ~ (destructor)

    ARROW,          // ->
    FAT_ARROW,      // =>
    DOT,            // .
    DOT_DOT,        // .. (range)
    COMMA,          // ,
    COLON,          // :
    SEMICOLON,      // ;
    QUESTION,       // ?

    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]

    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string text;
    size_t line;
    size_t column;
};

// AST Nodes Forward Declarations
struct ASTNode;
struct ExprNode;
struct StmtNode;

using NodePtr = std::shared_ptr<ASTNode>;
using ExprPtr = std::shared_ptr<ExprNode>;
using StmtPtr = std::shared_ptr<StmtNode>;

struct ASTNode {
    virtual ~ASTNode() = default;
    size_t line = 0;
};

struct ExprNode : ASTNode {};
struct StmtNode : ASTNode {};

// Expressions
struct LiteralExpr : ExprNode {
    TokenType lit_type;
    std::string value;
};

struct StringInterpolationExpr : ExprNode {
    struct Part {
        bool is_expr;
        std::string raw_text;
        ExprPtr expr;
    };
    std::vector<Part> parts;
};

struct IdentifierExpr : ExprNode {
    std::string name;
};

struct BinaryExpr : ExprNode {
    ExprPtr left;
    TokenType op;
    ExprPtr right;
};

struct UnaryExpr : ExprNode {
    TokenType op;
    ExprPtr operand;
    bool is_postfix = false;
};

struct TernaryExpr : ExprNode {
    ExprPtr condition;
    ExprPtr then_expr;
    ExprPtr else_expr;
};

struct CallExpr : ExprNode {
    ExprPtr callee;
    std::vector<ExprPtr> args;
    std::vector<std::string> template_args;
};

struct MemberAccessExpr : ExprNode {
    ExprPtr object;
    std::string member;
    bool is_arrow = false;
};

struct IndexExpr : ExprNode {
    ExprPtr object;
    ExprPtr index;
};

struct VectorLiteralExpr : ExprNode {
    std::vector<ExprPtr> elements;
};

struct MapLiteralExpr : ExprNode {
    std::vector<std::pair<ExprPtr, ExprPtr>> entries;
};

struct RangeExpr : ExprNode {
    ExprPtr start;
    ExprPtr stop;
    ExprPtr step;
};

// Statements
struct ExprStmt : StmtNode {
    ExprPtr expr;
};

struct VarDeclStmt : StmtNode {
    std::string name;
    std::string type_annot;
    bool is_mut = true;
    ExprPtr init_expr;
};

struct BlockStmt : StmtNode {
    std::vector<StmtPtr> statements;
};

struct IfStmt : StmtNode {
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;
};

struct WhileStmt : StmtNode {
    ExprPtr condition;
    StmtPtr body;
};

struct ForInStmt : StmtNode {
    std::string var_name;
    std::string index_name; // For enumerate / 2-variable unpack
    bool is_mut = false;
    ExprPtr iterable;
    StmtPtr body;
};

struct ForRangeStmt : StmtNode {
    std::string var_name;
    ExprPtr start;
    ExprPtr stop;
    ExprPtr step;
    StmtPtr body;
};

struct ReturnStmt : StmtNode {
    ExprPtr value;
};

struct BreakStmt : StmtNode {};
struct ContinueStmt : StmtNode {};

struct ParamDecl {
    std::string name;
    std::string type_annot;
    ExprPtr default_val;
    bool is_mut = false;
};

struct FnDeclStmt : StmtNode {
    std::string name;
    std::vector<std::string> generic_params;
    std::vector<ParamDecl> params;
    std::string return_type;
    StmtPtr body;
    ExprPtr arrow_body;
    bool is_method = false;
    bool is_operator = false;
    std::string op_symbol;
    bool is_destructor = false;
    bool is_constructor = false;
    bool is_override = false;
};

struct StructMember {
    std::string name;
    std::string type_annot;
    ExprPtr default_val;
    std::shared_ptr<FnDeclStmt> method;
    bool is_method = false;
};

struct StructDeclStmt : StmtNode {
    std::string name;
    std::string base_class;
    bool is_class = false;
    std::vector<std::string> generic_params;
    std::vector<StructMember> members;
};

struct EnumMember {
    std::string name;
    ExprPtr value;
};

struct EnumDeclStmt : StmtNode {
    std::string name;
    std::vector<EnumMember> members;
};

struct ImportStmt : StmtNode {
    std::string module_name;
};

struct RawCppStmt : StmtNode {
    std::string code;
};

struct CIncludeStmt : StmtNode {
    std::string header;
};

struct ProgramNode : ASTNode {
    std::vector<StmtPtr> statements;
};

} // namespace veyra
