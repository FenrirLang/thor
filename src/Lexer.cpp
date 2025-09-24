#include "Lexer.h"
#include <unordered_map>
#include <stdexcept>

Lexer::Lexer(const std::string& source) 
    : source(source), current(0), line(1), column(1) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        Token token = nextToken();
        if (token.type != TokenType::UNKNOWN) {
            tokens.push_back(token);
        }
    }
    
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line, column);
    return tokens;
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TokenType::EOF_TOKEN, "", line, column);
    }
    
    int tokenLine = line;
    int tokenColumn = column;
    char c = advance();
    
    switch (c) {
        case '(': return Token(TokenType::LEFT_PAREN, "(", tokenLine, tokenColumn);
        case ')': return Token(TokenType::RIGHT_PAREN, ")", tokenLine, tokenColumn);
        case '{': return Token(TokenType::LEFT_BRACE, "{", tokenLine, tokenColumn);
        case '}': return Token(TokenType::RIGHT_BRACE, "}", tokenLine, tokenColumn);
        case '[': return Token(TokenType::LEFT_BRACKET, "[", tokenLine, tokenColumn);
        case ']': return Token(TokenType::RIGHT_BRACKET, "]", tokenLine, tokenColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", tokenLine, tokenColumn);
        case ',': return Token(TokenType::COMMA, ",", tokenLine, tokenColumn);
        case '.': return Token(TokenType::DOT, ".", tokenLine, tokenColumn);
        case ':': return Token(TokenType::COLON, ":", tokenLine, tokenColumn);
        case '%': return Token(TokenType::PERCENT, "%", tokenLine, tokenColumn);
        case '+': return Token(TokenType::PLUS, "+", tokenLine, tokenColumn);
        case '*': return Token(TokenType::MULTIPLY, "*", tokenLine, tokenColumn);
        case '/':
            if (peek() == '/') {
                skipComment();
                return nextToken();
            }
            return Token(TokenType::DIVIDE, "/", tokenLine, tokenColumn);
        case '-':
            if (peek() == '>') {
                advance();
                return Token(TokenType::ARROW, "->", tokenLine, tokenColumn);
            }
            return Token(TokenType::MINUS, "-", tokenLine, tokenColumn);
        case '=':
            if (peek() == '=') {
                advance();
                return Token(TokenType::EQUAL, "==", tokenLine, tokenColumn);
            }
            return Token(TokenType::ASSIGN, "=", tokenLine, tokenColumn);
        case '!':
            if (peek() == '=') {
                advance();
                return Token(TokenType::NOT_EQUAL, "!=", tokenLine, tokenColumn);
            }
            return Token(TokenType::NOT, "!", tokenLine, tokenColumn);
        case '<': return Token(TokenType::LESS_THAN, "<", tokenLine, tokenColumn);
        case '>': return Token(TokenType::GREATER_THAN, ">", tokenLine, tokenColumn);
        case '&':
            if (peek() == '&') {
                advance();
                return Token(TokenType::AND, "&&", tokenLine, tokenColumn);
            }
            break;
        case '|':
            if (peek() == '|') {
                advance();
                return Token(TokenType::OR, "||", tokenLine, tokenColumn);
            }
            break;
        case '"':
            current--; column--; // Back up to include quote
            return makeString();
        case '\n':
            return Token(TokenType::NEWLINE, "\n", tokenLine, tokenColumn);
        default:
            if (isDigit(c)) {
                current--; column--; // Back up to include digit
                return makeNumber();
            }
            if (isAlpha(c)) {
                current--; column--; // Back up to include character
                return makeIdentifier();
            }
            break;
    }
    
    return Token(TokenType::UNKNOWN, std::string(1, c), tokenLine, tokenColumn);
}

char Lexer::peek(int offset) const {
    size_t index = current + offset;
    if (index >= source.length()) return '\0';
    return source[index];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    
    char c = source[current++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t') {
            advance();
        } else {
            break;
        }
    }
}

void Lexer::skipComment() {
    // Skip // comment
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

Token Lexer::makeString() {
    int tokenLine = line;
    int tokenColumn = column;
    
    advance(); // consume opening quote
    std::string value;
    
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\\') {
            advance(); // consume backslash
            if (!isAtEnd()) {
                char escaped = advance();
                switch (escaped) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    default: value += escaped; break;
                }
            }
        } else {
            value += advance();
        }
    }
    
    if (isAtEnd()) {
        throw std::runtime_error("Unterminated string at line " + std::to_string(tokenLine));
    }
    
    advance(); // consume closing quote
    return Token(TokenType::STRING, value, tokenLine, tokenColumn);
}

Token Lexer::makeNumber() {
    int tokenLine = line;
    int tokenColumn = column;
    std::string value;
    
    while (!isAtEnd() && isDigit(peek())) {
        value += advance();
    }
    
    return Token(TokenType::INTEGER, value, tokenLine, tokenColumn);
}

Token Lexer::makeIdentifier() {
    int tokenLine = line;
    int tokenColumn = column;
    std::string value;
    
    while (!isAtEnd() && isAlphaNumeric(peek())) {
        value += advance();
    }
    
    TokenType type = getKeywordType(value);
    return Token(type, value, tokenLine, tokenColumn);
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

TokenType Lexer::getKeywordType(const std::string& text) const {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"package", TokenType::PACKAGE},
        {"import", TokenType::IMPORT},
        {"func", TokenType::FUNC},
        {"return", TokenType::RETURN},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"int", TokenType::INT},
        {"string", TokenType::STRING_TYPE},
        {"boolean", TokenType::BOOLEAN_TYPE},
        {"void", TokenType::VOID_TYPE},
        {"true", TokenType::TRUE_VALUE},
        {"false", TokenType::FALSE_VALUE}
    };
    
    auto it = keywords.find(text);
    return (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}