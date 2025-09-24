#pragma once
#include "Token.h"
#include <string>
#include <vector>

class Lexer {
private:
    std::string source;
    size_t current;
    int line;
    int column;
    
    char peek(int offset = 0) const;
    char advance();
    void skipWhitespace();
    void skipComment();
    Token makeString();
    Token makeNumber();
    Token makeIdentifier();
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;
    TokenType getKeywordType(const std::string& text) const;
    
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();
    Token nextToken();
    bool isAtEnd() const;
};