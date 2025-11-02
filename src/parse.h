#pragma once

#include <cstdint>

#include "common_type.h"
#include "tokenizer.h"

enum class ASTKind {
    TranslationUnit,
    Function,
    Num,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    LogicalOr,
    LogicalAnd,
    BitOr,
    BitXor,
    BitAnd,
    Equal,
    NotEqual,
    LessThan,
    LessEqual,
    LShift,
    RShift,
    CompoundStmt,
    Return,
    VarDecl,
    Variable,
    Assign,
    If,
    While,
    Switch,
    Case,
    FunctionCall,
    Import,
    StringLiteral,
};

enum class ASTFlags : uint8_t {
    None = 0,
    Mutable = 1 << 0,
    Public = 1 << 1,
};

inline ASTFlags operator|(ASTFlags a, ASTFlags b) {
    return static_cast<ASTFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline ASTFlags operator&(ASTFlags a, ASTFlags b) {
    return static_cast<ASTFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline bool hasFlag(ASTFlags flags, ASTFlags flag) {
    return (flags & flag) != ASTFlags::None;
}

struct ASTNode {
    ASTKind kind;
    ASTIdx lhs;
    ASTIdx rhs;

    ASTFlags flags = ASTFlags::None;

    // For Num
    int32_t val;

    // For CompoundStmt or Function
    std::vector<ASTIdx> body;

    // For Function
    std::vector<ASTIdx> params;

    // For FunctionCall
    std::vector<ASTIdx> args;
    
    // For VarDecl
    std::string name;

    // For StringLiteral
    std::string str;

    // For if statement
    ASTIdx cond;
    ASTIdx thenBr;
    ASTIdx elseBr;

    SymbolIdx symIdx;

    bool isMut() const {
        return hasFlag(flags, ASTFlags::Mutable);
    }
    bool isPub() const {
        return hasFlag(flags, ASTFlags::Public);
    }
    void setMut(bool is_mut) {
        if(is_mut) {
            flags = flags | ASTFlags::Mutable;
        } else {
            flags = static_cast<ASTFlags>(static_cast<uint8_t>(flags) & ~static_cast<uint8_t>(ASTFlags::Mutable));
        }
    }
    void setPub(bool is_pub) {
        if(is_pub) {
            flags = flags | ASTFlags::Public;
        } else {
            flags = static_cast<ASTFlags>(static_cast<uint8_t>(flags) & ~static_cast<uint8_t>(ASTFlags::Public));
        }
    }
};

class Parser{
public:
    Parser(Tokenizer::TokenStream& ts) : ts(ts) {}
    ASTIdx parseFile();
    void printAST(ASTIdx idx);
    ASTNode& getAST(ASTIdx idx) { return nodes[idx]; }
private:
    Tokenizer::TokenStream& ts;
    std::vector<ASTNode> nodes;
    ASTIdx functionDef(bool is_pub = false);
    ASTIdx compoundStmt();
    ASTIdx stmt();
    ASTIdx expr();
    ASTIdx logical_or();
    ASTIdx logical_and();
    ASTIdx bitwise_or();
    ASTIdx bitwise_xor();
    ASTIdx bitwise_and();
    ASTIdx equality();
    ASTIdx relational();
    ASTIdx shift();
    ASTIdx add();
    ASTIdx mul();
    ASTIdx unary();
    ASTIdx primary();
    ASTIdx newNode(ASTKind kind, ASTIdx lhs, ASTIdx rhs);
    ASTIdx newNodeNum(int32_t val);

    int32_t depth = 0;
    void printStmt(ASTIdx idx);
    void printExpr(ASTIdx idx);
    void printAST(const ASTNode& node, int32_t depth = 0) const;
};

