#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <optional>

#include "common_type.h"

enum class TokenKind {
    Num,
    Add,         // +
    Sub,         // -
    Mul,         // *
    Div,         // /
    Mod,         // %
    Equal,       // =
    EqualEqual,  // ==
    NotEqual,    // !=
    LessThan,    // <
    LessEqual,   // <=
    LShift,      // <<
    RShift,      // >>
    And,         // &
    Hat,         // ^
    Or,          // |
    AndAnd,      // &&
    OrOr,        // ||
    LParen,      // (
    RParen,      // )
    LBrace,      // {
    RBrace,      // }
    Semicolon,   // ;
    Comma,       // ,
    EqualArrow,  // =>
    Return,      // "return"
    Let,         // "let"
    Mut,         // "mut"
    If,          // "if"
    Else,        // "else"
    While,       // "while"
    Switch,      // "switch"
    Fn,          // "fn"
    Import,      // "import"
    Pub,         // "pub"
    Ident,       // Identifier
    StringLiteral, // String literal
    // =========== for tnlib ===========
    Tnlib,
    Module,
    End,         // "end"
    // =========== end of tnlib ===========
    Eof,
};

struct Token{
    TokenKind kind;
    int32_t val;
    char* pos;
    int32_t len;
    std::string str;
};

class Tokenizer {
public:
    class TokenStream {
        friend class Tokenizer;
        std::vector<Token> tokens;
        void clear() { tokens.clear(); }
        void addToken(const Token& token) { tokens.push_back(token); }
        void addToken(TokenKind kind, char* pos);
        Token& getTop() { return tokens.back(); }
    public:
        TokenIdx idx;
        Token getToken(TokenIdx idx) { return tokens[idx]; }
        void reset() { idx = 0; }
        bool consume(TokenKind kind);
        void expect(TokenKind kind);
        int32_t expectNum();
        TokenIdx expectIdent();
        TokenIdx expectStringLiteral();
        std::optional<TokenIdx> consumeIdent();
        bool peekKind(TokenKind kind, TokenIdx offset = 0);
    };
    TokenStream& scan(char* p);
    Tokenizer(bool for_tnlib = false) : for_tnlib(for_tnlib) {}
    bool for_tnlib;
    void printTokens(TokenStream& ts);
private:
    static std::map<std::string, TokenKind> keyword_map;
    static std::map<std::string, TokenKind> tnlib_keyword_map;
    //TokenStream ts;
    TokenKind checkKeyword(char* start, uint32_t len);
    TokenKind checkKeywordTnlib(char* start, uint32_t len);
    bool is_ident1(char c);
    bool is_ident2(char c);
    void printTokenKind(TokenKind kind);
};
