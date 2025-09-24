#pragma once
#include "AST.h"
#include "Token.h"
#include <vector>
#include <memory>

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    Token& peek(int offset = 0);
    Token& advance();
    bool check(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    bool isAtEnd() const;
    void consume(TokenType type, const std::string& message);
    
    // Parsing methods
    std::shared_ptr<Type> parseType();
    std::shared_ptr<Expression> parseExpression();
    std::shared_ptr<Expression> parseAssignment();
    std::shared_ptr<Expression> parseLogicalOr();
    std::shared_ptr<Expression> parseLogicalAnd();
    std::shared_ptr<Expression> parseEquality();
    std::shared_ptr<Expression> parseComparison();
    std::shared_ptr<Expression> parseTerm();
    std::shared_ptr<Expression> parseFactor();
    std::shared_ptr<Expression> parseUnary();
    std::shared_ptr<Expression> parseCall();
    std::shared_ptr<Expression> parsePrimary();
    
    std::shared_ptr<Statement> parseStatement();
    std::shared_ptr<Statement> parseExpressionStatement();
    std::shared_ptr<VariableDeclaration> parseVariableDeclaration();
    std::shared_ptr<ConstDeclaration> parseConstDeclaration();
    std::shared_ptr<BlockStatement> parseBlock();
    std::shared_ptr<IfStatement> parseIfStatement();
    std::shared_ptr<WhileStatement> parseWhileStatement();
    std::shared_ptr<ReturnStatement> parseReturnStatement();
    std::shared_ptr<FunctionDeclaration> parseFunctionDeclaration();
    std::shared_ptr<PackageDeclaration> parsePackageDeclaration();
    std::shared_ptr<ImportDeclaration> parseImportDeclaration();
    
public:
    Parser(std::vector<Token> tokens);
    std::shared_ptr<Program> parse();
};