#pragma once
#include <string>
#include <unordered_map>

namespace Thor {

enum class TokenType {
    // Literals
    NUMBER,
    STRING,
    IDENTIFIER,
    
    // Keywords
    INT,
    FLOAT,
    STRING_KW,
    BOOL,
    VOID,
    IF,
    ELSE,
    WHILE,
    FOR,
    RETURN,
    TRUE,
    FALSE,
    IMPORT,
    EXTERN,
    
    // Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    AND,
    OR,
    NOT,
    
    // Punctuation
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
    
    // Special
    NAMESPACE_SEP,  // ::
    EOF_TOKEN,
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t = TokenType::INVALID, const std::string& v = "", int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
};

class TokenInfo {
public:
    static const std::unordered_map<std::string, TokenType> keywords;
    static std::string toString(TokenType type);
    static bool isKeyword(const std::string& text);
    static TokenType getKeywordType(const std::string& text);
};

} // namespace Thor