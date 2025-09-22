#include "Lexer.h"
#include <cctype>
#include <stdexcept>

namespace Thor {

Lexer::Lexer(const std::string& src) : source(src), position(0), line(1), column(1) {}

char Lexer::currentChar() const {
    if (position >= source.length()) {
        return '\0';
    }
    return source[position];
}

char Lexer::peekChar(size_t offset) const {
    size_t pos = position + offset;
    if (pos >= source.length()) {
        return '\0';
    }
    return source[pos];
}

void Lexer::advance() {
    if (position < source.length()) {
        if (currentChar() == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
    }
}

void Lexer::skipWhitespace() {
    while (std::isspace(currentChar())) {
        advance();
    }
}

void Lexer::skipLineComment() {
    // Skip // comments
    while (currentChar() != '\n' && currentChar() != '\0') {
        advance();
    }
}

void Lexer::skipBlockComment() {
    // Skip /* */ comments
    advance(); // skip '/'
    advance(); // skip '*'
    
    while (currentChar() != '\0') {
        if (currentChar() == '*' && peekChar() == '/') {
            advance(); // skip '*'
            advance(); // skip '/'
            break;
        }
        advance();
    }
}

Token Lexer::readNumber() {
    std::string number;
    int startLine = static_cast<int>(line);
    int startColumn = static_cast<int>(column);
    
    bool hasDecimal = false;
    
    while (std::isdigit(currentChar()) || (currentChar() == '.' && !hasDecimal)) {
        if (currentChar() == '.') {
            hasDecimal = true;
        }
        number += currentChar();
        advance();
    }
    
    return Token(TokenType::NUMBER, number, startLine, startColumn);
}

Token Lexer::readString() {
    std::string str;
    int startLine = static_cast<int>(line);
    int startColumn = static_cast<int>(column);
    
    advance(); // skip opening quote
    
    while (currentChar() != '"' && currentChar() != '\0') {
        if (currentChar() == '\\') {
            advance();
            switch (currentChar()) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                default: str += currentChar(); break;
            }
        } else {
            str += currentChar();
        }
        advance();
    }
    
    if (currentChar() == '"') {
        advance(); // skip closing quote
    }
    
    return Token(TokenType::STRING, str, startLine, startColumn);
}

Token Lexer::readIdentifier() {
    std::string identifier;
    int startLine = static_cast<int>(line);
    int startColumn = static_cast<int>(column);
    
    while (std::isalnum(currentChar()) || currentChar() == '_') {
        identifier += currentChar();
        advance();
    }
    
    TokenType type = TokenInfo::isKeyword(identifier) ? 
                     TokenInfo::getKeywordType(identifier) : 
                     TokenType::IDENTIFIER;
    
    return Token(type, identifier, startLine, startColumn);
}

Token Lexer::readOperator() {
    int startLine = static_cast<int>(line);
    int startColumn = static_cast<int>(column);
    char c = currentChar();
    
    advance();
    
    switch (c) {
        case '+': return Token(TokenType::PLUS, "+", startLine, startColumn);
        case '-': return Token(TokenType::MINUS, "-", startLine, startColumn);
        case '*': return Token(TokenType::MULTIPLY, "*", startLine, startColumn);
        case '/': return Token(TokenType::DIVIDE, "/", startLine, startColumn);
        case '%': return Token(TokenType::MODULO, "%", startLine, startColumn);
        case '(':  return Token(TokenType::LEFT_PAREN, "(", startLine, startColumn);
        case ')': return Token(TokenType::RIGHT_PAREN, ")", startLine, startColumn);
        case '{': return Token(TokenType::LEFT_BRACE, "{", startLine, startColumn);
        case '}': return Token(TokenType::RIGHT_BRACE, "}", startLine, startColumn);
        case '[': return Token(TokenType::LEFT_BRACKET, "[", startLine, startColumn);
        case ']': return Token(TokenType::RIGHT_BRACKET, "]", startLine, startColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", startLine, startColumn);
        case ',': return Token(TokenType::COMMA, ",", startLine, startColumn);
        case '.': return Token(TokenType::DOT, ".", startLine, startColumn);
        case '=':
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::EQUAL, "==", startLine, startColumn);
            }
            return Token(TokenType::ASSIGN, "=", startLine, startColumn);
        case '!':
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::NOT_EQUAL, "!=", startLine, startColumn);
            }
            return Token(TokenType::NOT, "!", startLine, startColumn);
        case '<':
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::LESS_EQUAL, "<=", startLine, startColumn);
            }
            return Token(TokenType::LESS, "<", startLine, startColumn);
        case '>':
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::GREATER_EQUAL, ">=", startLine, startColumn);
            }
            return Token(TokenType::GREATER, ">", startLine, startColumn);
        case '&':
            if (currentChar() == '&') {
                advance();
                return Token(TokenType::AND, "&&", startLine, startColumn);
            }
            break;
        case '|':
            if (currentChar() == '|') {
                advance();
                return Token(TokenType::OR, "||", startLine, startColumn);
            }
            break;
        case ':':
            if (currentChar() == ':') {
                advance();
                return Token(TokenType::NAMESPACE_SEP, "::", startLine, startColumn);
            }
            return Token(TokenType::COLON, ":", startLine, startColumn);
    }
    
    return Token(TokenType::INVALID, std::string(1, c), startLine, startColumn);
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TokenType::EOF_TOKEN, "", static_cast<int>(line), static_cast<int>(column));
    }
    
    char c = currentChar();
    
    // Handle comments
    if (c == '/' && peekChar() == '/') {
        skipLineComment();
        return nextToken();
    }
    
    if (c == '/' && peekChar() == '*') {
        skipBlockComment();
        return nextToken();
    }
    
    // Numbers
    if (std::isdigit(c)) {
        return readNumber();
    }
    
    // Strings
    if (c == '"') {
        return readString();
    }
    
    // Identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        return readIdentifier();
    }
    
    // Operators and punctuation
    return readOperator();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        Token token = nextToken();
        if (token.type != TokenType::INVALID) {
            tokens.push_back(token);
        }
        
        if (token.type == TokenType::EOF_TOKEN) {
            break;
        }
    }
    
    return tokens;
}

bool Lexer::isAtEnd() const {
    return position >= source.length();
}

} // namespace Thor