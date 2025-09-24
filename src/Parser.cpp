#include "Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : tokens(tokens), current(0) {}

std::shared_ptr<Program> Parser::parse() {
    auto program = std::make_shared<Program>();
    
    // Skip newlines at the beginning
    while (match({TokenType::NEWLINE})) {}
    
    // Parse package declaration
    if (check(TokenType::PACKAGE)) {
        program->package = parsePackageDeclaration();
        while (match({TokenType::NEWLINE})) {}
    }
    
    // Parse imports
    while (check(TokenType::IMPORT)) {
        program->imports.push_back(parseImportDeclaration());
        while (match({TokenType::NEWLINE})) {}
    }
    
    // Parse top-level statements
    while (!isAtEnd()) {
        if (match({TokenType::NEWLINE})) {
            continue;
        }
        program->statements.push_back(parseStatement());
        while (match({TokenType::NEWLINE})) {}
    }
    
    return program;
}

Token& Parser::peek(int offset) {
    size_t index = current + offset;
    if (index >= tokens.size()) {
        return tokens.back(); // Return EOF token
    }
    return tokens[index];
}

Token& Parser::advance() {
    if (!isAtEnd()) current++;
    return peek(-1);
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::isAtEnd() const {
    return current >= tokens.size() || tokens[current].type == TokenType::EOF_TOKEN;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return;
    }
    
    Token& token = peek();
    throw std::runtime_error(message + " at line " + std::to_string(token.line) + 
                           ", got '" + token.value + "'");
}

std::shared_ptr<Type> Parser::parseType() {
    if (match({TokenType::VOID_TYPE})) {
        return Type::createVoid();
    } else if (match({TokenType::INT})) {
        return Type::createInt();
    } else if (match({TokenType::STRING_TYPE})) {
        return Type::createString();
    } else if (match({TokenType::BOOLEAN_TYPE})) {
        return Type::createBoolean();
    } else if (check(TokenType::IDENTIFIER)) {
        // For array types like "string[]"
        std::string typeName = advance().value;
        if (match({TokenType::LEFT_BRACKET})) {
            consume(TokenType::RIGHT_BRACKET, "Expected ']' after '['");
            if (typeName == "string") {
                return Type::createArray(Type::createString());
            } else if (typeName == "int") {
                return Type::createArray(Type::createInt());
            } else if (typeName == "boolean") {
                return Type::createArray(Type::createBoolean());
            }
        }
        // Handle other custom types if needed
        throw std::runtime_error("Unknown type: " + typeName);
    }
    
    throw std::runtime_error("Expected type");
}

std::shared_ptr<Expression> Parser::parseExpression() {
    return parseAssignment();
}

std::shared_ptr<Expression> Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    
    if (match({TokenType::ASSIGN})) {
        std::string op = peek(-1).value;
        auto value = parseAssignment();
        return std::make_shared<BinaryExpression>(expr, op, value);
    }
    
    return expr;
}

std::shared_ptr<Expression> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (match({TokenType::OR})) {
        std::string op = peek(-1).value;
        auto right = parseLogicalAnd();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

std::shared_ptr<Expression> Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (match({TokenType::AND})) {
        std::string op = peek(-1).value;
        auto right = parseEquality();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

std::shared_ptr<Expression> Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (match({TokenType::EQUAL, TokenType::NOT_EQUAL})) {
        std::string op = peek(-1).value;
        auto right = parseComparison();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

std::shared_ptr<Expression> Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match({TokenType::GREATER_THAN, TokenType::LESS_THAN})) {
        std::string op = peek(-1).value;
        auto right = parseTerm();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

std::shared_ptr<Expression> Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        std::string op = peek(-1).value;
        auto right = parseFactor();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

std::shared_ptr<Expression> Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match({TokenType::DIVIDE, TokenType::MULTIPLY, TokenType::MODULO})) {
        std::string op = peek(-1).value;
        auto right = parseUnary();
        expr = std::make_shared<BinaryExpression>(expr, op, right);
    }
    
    return expr;
}

std::shared_ptr<Expression> Parser::parseUnary() {
    if (match({TokenType::NOT, TokenType::MINUS})) {
        std::string op = peek(-1).value;
        auto right = parseUnary();
        return std::make_shared<UnaryExpression>(op, right);
    }
    
    return parseCall();
}

std::shared_ptr<Expression> Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            std::vector<std::shared_ptr<Expression>> arguments;
            
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    arguments.push_back(parseExpression());
                } while (match({TokenType::COMMA}));
            }
            
            consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
            expr = std::make_shared<CallExpression>(expr, arguments);
        } else if (match({TokenType::DOT})) {
            consume(TokenType::IDENTIFIER, "Expected property name after '.'");
            std::string property = peek(-1).value;
            expr = std::make_shared<MemberExpression>(expr, property);
        } else {
            break;
        }
    }
    
    return expr;
}

std::shared_ptr<Expression> Parser::parsePrimary() {
    if (match({TokenType::TRUE_VALUE})) {
        return std::make_shared<LiteralExpression>("true", LiteralExpression::BOOLEAN);
    }
    
    if (match({TokenType::FALSE_VALUE})) {
        return std::make_shared<LiteralExpression>("false", LiteralExpression::BOOLEAN);
    }
    
    if (match({TokenType::INTEGER})) {
        return std::make_shared<LiteralExpression>(peek(-1).value, LiteralExpression::INTEGER);
    }
    
    if (match({TokenType::STRING})) {
        std::string value = peek(-1).value;
        
        // Check if this is a format string (contains % followed by [)
        if (check(TokenType::PERCENT)) {
            advance(); // consume %
            consume(TokenType::LEFT_BRACKET, "Expected '[' after '%'");
            
            std::vector<std::shared_ptr<Expression>> args;
            if (!check(TokenType::RIGHT_BRACKET)) {
                do {
                    args.push_back(parseExpression());
                } while (match({TokenType::COMMA}));
            }
            
            consume(TokenType::RIGHT_BRACKET, "Expected ']' after format arguments");
            return std::make_shared<FormatStringExpression>(value, args);
        }
        
        return std::make_shared<LiteralExpression>(value, LiteralExpression::STRING);
    }
    
    if (match({TokenType::IDENTIFIER})) {
        return std::make_shared<IdentifierExpression>(peek(-1).value);
    }
    
    if (match({TokenType::LEFT_PAREN})) {
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    if (match({TokenType::LEFT_BRACKET})) {
        std::vector<std::shared_ptr<Expression>> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(parseExpression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Expected ']' after array elements");
        return std::make_shared<ArrayExpression>(elements);
    }
    
    Token& token = peek();
    throw std::runtime_error("Unexpected token '" + token.value + "' at line " + std::to_string(token.line));
}

std::shared_ptr<Statement> Parser::parseStatement() {
    if (check(TokenType::FUNC)) {
        return parseFunctionDeclaration();
    }
    
    if (check(TokenType::LEFT_BRACE)) {
        return parseBlock();
    }
    
    if (match({TokenType::IF})) {
        return parseIfStatement();
    }
    
    if (match({TokenType::WHILE})) {
        return parseWhileStatement();
    }
    
    if (match({TokenType::RETURN})) {
        return parseReturnStatement();
    }
    
    // Check for variable declaration - type followed by identifier
    if (check(TokenType::INT) || check(TokenType::STRING_TYPE) || check(TokenType::BOOLEAN_TYPE)) {
        return parseVariableDeclaration();
    }
    
    return parseExpressionStatement();
}

std::shared_ptr<Statement> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return std::make_shared<ExpressionStatement>(expr);
}

std::shared_ptr<VariableDeclaration> Parser::parseVariableDeclaration() {
    std::shared_ptr<Type> type = parseType();
    consume(TokenType::IDENTIFIER, "Expected variable name");
    std::string name = peek(-1).value;
    
    std::shared_ptr<Expression> initializer = nullptr;
    if (match({TokenType::ASSIGN})) {
        initializer = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_shared<VariableDeclaration>(name, type, initializer);
}

std::shared_ptr<BlockStatement> Parser::parseBlock() {
    consume(TokenType::LEFT_BRACE, "Expected '{'");
    
    std::vector<std::shared_ptr<Statement>> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match({TokenType::NEWLINE})) {
            continue;
        }
        statements.push_back(parseStatement());
        while (match({TokenType::NEWLINE})) {}
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}'");
    return std::make_shared<BlockStatement>(statements);
}

std::shared_ptr<IfStatement> Parser::parseIfStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");
    
    auto thenBranch = parseStatement();
    std::shared_ptr<Statement> elseBranch = nullptr;
    
    // Skip newlines before else
    while (match({TokenType::NEWLINE})) {}
    
    if (match({TokenType::ELSE})) {
        elseBranch = parseStatement();
    }
    
    return std::make_shared<IfStatement>(condition, thenBranch, elseBranch);
}

std::shared_ptr<WhileStatement> Parser::parseWhileStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");
    
    auto body = parseStatement();
    return std::make_shared<WhileStatement>(condition, body);
}

std::shared_ptr<ReturnStatement> Parser::parseReturnStatement() {
    std::shared_ptr<Expression> value = nullptr;
    
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after return value");
    return std::make_shared<ReturnStatement>(value);
}

std::shared_ptr<FunctionDeclaration> Parser::parseFunctionDeclaration() {
    consume(TokenType::FUNC, "Expected 'func'");
    consume(TokenType::IDENTIFIER, "Expected function name");
    std::string name = peek(-1).value;
    
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    
    std::vector<Parameter> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            // Parse parameter type (which could be an array type)
            std::shared_ptr<Type> paramType;
            if (match({TokenType::INT})) {
                paramType = Type::createInt();
            } else if (match({TokenType::STRING_TYPE})) {
                paramType = Type::createString();
            } else if (match({TokenType::BOOLEAN_TYPE})) {
                paramType = Type::createBoolean();
            } else {
                throw std::runtime_error("Expected parameter type");
            }
            
            // Check for array type
            if (match({TokenType::LEFT_BRACKET})) {
                consume(TokenType::RIGHT_BRACKET, "Expected ']' after '['");
                paramType = Type::createArray(paramType);
            }
            
            consume(TokenType::IDENTIFIER, "Expected parameter name");
            std::string paramName = peek(-1).value;
            parameters.emplace_back(paramName, paramType);
        } while (match({TokenType::COMMA}));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
    consume(TokenType::ARROW, "Expected '->' after parameter list");
    
    auto returnType = parseType();
    auto body = parseBlock();
    
    return std::make_shared<FunctionDeclaration>(name, parameters, returnType, body);
}

std::shared_ptr<PackageDeclaration> Parser::parsePackageDeclaration() {
    consume(TokenType::PACKAGE, "Expected 'package'");
    consume(TokenType::IDENTIFIER, "Expected package name");
    std::string name = peek(-1).value;
    consume(TokenType::SEMICOLON, "Expected ';' after package declaration");
    
    return std::make_shared<PackageDeclaration>(name);
}

std::shared_ptr<ImportDeclaration> Parser::parseImportDeclaration() {
    consume(TokenType::IMPORT, "Expected 'import'");
    consume(TokenType::STRING, "Expected module name");
    std::string module = peek(-1).value;
    consume(TokenType::SEMICOLON, "Expected ';' after import declaration");
    
    return std::make_shared<ImportDeclaration>(module);
}