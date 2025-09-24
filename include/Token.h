#pragma once
#include <string>
#include <vector>

enum class TokenType {
    // Literals
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    IDENTIFIER,
    
    // Keywords
    PACKAGE,
    IMPORT,
    FUNC,
    RETURN,
    IF,
    ELSE,
    WHILE,
    CONST,
    INT,
    FLOAT_TYPE,
    STRING_TYPE,
    BOOLEAN_TYPE,
    VOID_TYPE,
    TRUE_VALUE,
    FALSE_VALUE,
    
    // Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    AND,
    OR,
    NOT,
    MODULO,
    AMPERSAND,
    
    // Delimiters
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    SEMICOLON,
    COMMA,
    DOT,
    COLON,
    ARROW,
    PERCENT,
    
    // Special
    NEWLINE,
    EOF_TOKEN,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
};