#pragma once
#include "Token.h"
#include <string>
#include <vector>

namespace Thor {

class Lexer {
private:
    std::string source;
    size_t position;
    size_t line;
    size_t column;
    
    char currentChar() const;
    char peekChar(size_t offset = 1) const;
    void advance();
    void skipWhitespace();
    void skipLineComment();
    void skipBlockComment();
    
    Token readNumber();
    Token readString();
    Token readIdentifier();
    Token readOperator();
    
public:
    Lexer(const std::string& src);
    
    std::vector<Token> tokenize();
    Token nextToken();
    
    bool isAtEnd() const;
    int getCurrentLine() const { return static_cast<int>(line); }
    int getCurrentColumn() const { return static_cast<int>(column); }
};

} // namespace Thor