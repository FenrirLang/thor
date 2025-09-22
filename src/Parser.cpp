#include "Parser.h"
#include <iostream>

namespace Thor {

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

Token& Parser::currentToken() {
    if (current >= tokens.size()) {
        static Token eofToken(TokenType::EOF_TOKEN);
        return eofToken;
    }
    return tokens[current];
}

const Token& Parser::currentToken() const {
    if (current >= tokens.size()) {
        static Token eofToken(TokenType::EOF_TOKEN);
        return eofToken;
    }
    return tokens[current];
}

Token& Parser::peekToken(size_t offset) {
    size_t pos = current + offset;
    if (pos >= tokens.size()) {
        static Token eofToken(TokenType::EOF_TOKEN);
        return eofToken;
    }
    return tokens[pos];
}

void Parser::advance() {
    if (current < tokens.size()) {
        current++;
    }
}

bool Parser::isAtEnd() const {
    return current >= tokens.size() || currentToken().type == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return currentToken().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        Token token = currentToken();
        advance();
        return token;
    }
    
    throw ParseError(message + " at line " + std::to_string(currentToken().line));
}

Type Parser::parseType() {
    if (match(TokenType::INT)) return Type::INT;
    if (match(TokenType::FLOAT)) return Type::FLOAT;
    if (match(TokenType::STRING_KW)) return Type::STRING;
    if (match(TokenType::BOOL)) return Type::BOOL;
    if (match(TokenType::VOID)) {
        // Check for void* syntax
        if (match(TokenType::MULTIPLY)) {
            return Type::VOID_PTR;
        }
        return Type::VOID;
    }
    
    throw ParseError("Expected type at line " + std::to_string(currentToken().line));
}

std::unique_ptr<Expression> Parser::expression() {
    return logicalOr();
}

std::unique_ptr<Expression> Parser::logicalOr() {
    auto expr = logicalAnd();
    
    while (match(TokenType::OR)) {
        std::string op = "||";
        auto right = logicalAnd();
        expr = std::make_unique<BinaryOperation>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::logicalAnd() {
    auto expr = equality();
    
    while (match(TokenType::AND)) {
        std::string op = "&&";
        auto right = equality();
        expr = std::make_unique<BinaryOperation>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::equality() {
    auto expr = comparison();
    
    while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL)) {
        std::string op = tokens[current - 1].value;
        auto right = comparison();
        expr = std::make_unique<BinaryOperation>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::comparison() {
    auto expr = term();
    
    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) || 
           match(TokenType::LESS) || match(TokenType::LESS_EQUAL)) {
        std::string op = tokens[current - 1].value;
        auto right = term();
        expr = std::make_unique<BinaryOperation>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::term() {
    auto expr = factor();
    
    while (match(TokenType::MINUS) || match(TokenType::PLUS)) {
        std::string op = tokens[current - 1].value;
        auto right = factor();
        expr = std::make_unique<BinaryOperation>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::factor() {
    auto expr = unary();
    
    while (match(TokenType::DIVIDE) || match(TokenType::MULTIPLY) || match(TokenType::MODULO)) {
        std::string op = tokens[current - 1].value;
        auto right = unary();
        expr = std::make_unique<BinaryOperation>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::unary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS)) {
        std::string op = tokens[current - 1].value;
        auto right = unary();
        return std::make_unique<UnaryOperation>(op, std::move(right));
    }
    
    return call();
}

std::unique_ptr<Expression> Parser::call() {
    auto expr = primary();
    
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            // This is a function call
            if (auto identifier = dynamic_cast<Identifier*>(expr.get())) {
                auto call = std::make_unique<FunctionCall>(identifier->name);
                
                // Parse arguments
                if (!check(TokenType::RIGHT_PAREN)) {
                    do {
                        call->addArgument(expression());
                    } while (match(TokenType::COMMA));
                }
                
                consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
                expr = std::move(call);
            } else {
                throw ParseError("Invalid function call at line " + std::to_string(currentToken().line));
            }
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::primary() {
    if (match(TokenType::TRUE)) {
        return std::make_unique<BoolLiteral>(true);
    }
    
    if (match(TokenType::FALSE)) {
        return std::make_unique<BoolLiteral>(false);
    }
    
    if (match(TokenType::NUMBER)) {
        return std::make_unique<NumberLiteral>(tokens[current - 1].value);
    }
    
    if (match(TokenType::STRING)) {
        return std::make_unique<StringLiteral>(tokens[current - 1].value);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        std::string name = tokens[current - 1].value;
        
        // Handle namespace syntax (e.g., std::println)
        if (match(TokenType::NAMESPACE_SEP)) {
            std::string memberName = consume(TokenType::IDENTIFIER, "Expected identifier after '::'").value;
            name = name + "::" + memberName;
        }
        
        return std::make_unique<Identifier>(name);
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw ParseError("Expected expression at line " + std::to_string(currentToken().line));
}

std::unique_ptr<Statement> Parser::statement() {
    if (match(TokenType::IMPORT)) {
        return importStatement();
    }
    
    if (match(TokenType::EXTERN)) {
        return externDeclaration();
    }
    
    if (check(TokenType::INT) || check(TokenType::FLOAT) || check(TokenType::STRING_KW) || 
        check(TokenType::BOOL) || check(TokenType::VOID)) {
        return declaration();
    }
    
    if (match(TokenType::IF)) {
        return ifStatement();
    }
    
    if (match(TokenType::WHILE)) {
        return whileStatement();
    }
    
    if (match(TokenType::RETURN)) {
        return returnStatement();
    }
    
    if (match(TokenType::LEFT_BRACE)) {
        return block();
    }
    
    return expressionStatement();
}

std::unique_ptr<Statement> Parser::declaration() {
    Type type = parseType();
    
    std::string name = consume(TokenType::IDENTIFIER, "Expected identifier").value;
    
    // Check if this is a function declaration
    if (check(TokenType::LEFT_PAREN)) {
        // Function declaration
        advance(); // consume '('
        
        auto func = std::make_unique<FunctionDeclaration>(type, name);
        
        // Parse parameters
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                Type paramType = parseType();
                std::string paramName = consume(TokenType::IDENTIFIER, "Expected parameter name").value;
                func->addParameter(paramType, paramName);
            } while (match(TokenType::COMMA));
        }
        
        consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
        
        if (match(TokenType::LEFT_BRACE)) {
            func->setBody(std::unique_ptr<Block>(static_cast<Block*>(block().release())));
        } else {
            consume(TokenType::SEMICOLON, "Expected ';' or '{' after function declaration");
        }
        
        return func;
    } else {
        // Variable declaration
        std::unique_ptr<Expression> initializer = nullptr;
        
        if (match(TokenType::ASSIGN)) {
            initializer = expression();
        }
        
        consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
        
        return std::make_unique<VariableDeclaration>(type, name, std::move(initializer));
    }
}

std::unique_ptr<Statement> Parser::importStatement() {
    std::string moduleName = consume(TokenType::STRING, "Expected module name after 'import'").value;
    consume(TokenType::SEMICOLON, "Expected ';' after import statement");
    
    return std::make_unique<ImportStatement>(moduleName);
}

std::unique_ptr<Statement> Parser::externDeclaration() {
    // Parse return type
    Type returnType = parseType();
    
    // Parse function name
    std::string functionName = consume(TokenType::IDENTIFIER, "Expected function name").value;
    
    // Create extern declaration
    auto externDecl = std::make_unique<ExternDeclaration>(returnType, functionName);
    
    // Parse parameters
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Type paramType = parseType();
            std::string paramName = consume(TokenType::IDENTIFIER, "Expected parameter name").value;
            externDecl->addParameter(paramType, paramName);
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
    consume(TokenType::SEMICOLON, "Expected ';' after extern declaration");
    
    return std::move(externDecl);
}

std::unique_ptr<Statement> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");
    
    auto thenStmt = statement();
    std::unique_ptr<Statement> elseStmt = nullptr;
    
    if (match(TokenType::ELSE)) {
        elseStmt = statement();
    }
    
    return std::make_unique<IfStatement>(std::move(condition), std::move(thenStmt), std::move(elseStmt));
}

std::unique_ptr<Statement> Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");
    
    auto body = statement();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Statement> Parser::returnStatement() {
    std::unique_ptr<Expression> value = nullptr;
    
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after return value");
    
    return std::make_unique<ReturnStatement>(std::move(value));
}

std::unique_ptr<Statement> Parser::expressionStatement() {
    // Check for assignment
    if (check(TokenType::IDENTIFIER) && peekToken().type == TokenType::ASSIGN) {
        std::string name = currentToken().value;
        advance(); // consume identifier
        advance(); // consume '='
        
        auto value = expression();
        consume(TokenType::SEMICOLON, "Expected ';' after assignment");
        
        return std::make_unique<Assignment>(name, std::move(value));
    }
    
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<Block> Parser::block() {
    auto blockStmt = std::make_unique<Block>();
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        blockStmt->addStatement(statement());
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block");
    
    return blockStmt;
}

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    
    while (!isAtEnd()) {
        try {
            auto stmt = statement();
            if (stmt) {
                program->addStatement(std::move(stmt));
            }
        } catch (const ParseError& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            // Simple error recovery: skip to next statement
            while (!isAtEnd() && !check(TokenType::SEMICOLON) && !check(TokenType::RIGHT_BRACE)) {
                advance();
            }
            if (match(TokenType::SEMICOLON)) {
                // Continue parsing
            }
        }
    }
    
    return program;
}

} // namespace Thor