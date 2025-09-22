#pragma once
#include "Token.h"
#include "AST.h"
#include <vector>
#include <memory>
#include <stdexcept>

namespace Thor {

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    Token& currentToken();
    const Token& currentToken() const;
    Token& peekToken(size_t offset = 1);
    void advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);
    
    Type parseType();
    
    std::unique_ptr<Expression> expression();
    std::unique_ptr<Expression> logicalOr();
    std::unique_ptr<Expression> logicalAnd();
    std::unique_ptr<Expression> equality();
    std::unique_ptr<Expression> comparison();
    std::unique_ptr<Expression> term();
    std::unique_ptr<Expression> factor();
    std::unique_ptr<Expression> unary();
    std::unique_ptr<Expression> call();
    std::unique_ptr<Expression> primary();
    
    std::unique_ptr<Statement> statement();
    std::unique_ptr<Statement> declaration();
    std::unique_ptr<Statement> importStatement();
    std::unique_ptr<Statement> externDeclaration();
    std::unique_ptr<Statement> variableDeclaration();
    std::unique_ptr<Statement> functionDeclaration();
    std::unique_ptr<Statement> ifStatement();
    std::unique_ptr<Statement> whileStatement();
    std::unique_ptr<Statement> returnStatement();
    std::unique_ptr<Statement> expressionStatement();
    std::unique_ptr<Block> block();
    
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<Program> parse();
};

} // namespace Thor