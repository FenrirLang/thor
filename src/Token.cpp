#include "Token.h"

namespace Thor {

const std::unordered_map<std::string, TokenType> TokenInfo::keywords = {
    {"int", TokenType::INT},
    {"float", TokenType::FLOAT},
    {"string", TokenType::STRING_KW},
    {"bool", TokenType::BOOL},
    {"void", TokenType::VOID},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"import", TokenType::IMPORT},
    {"extern", TokenType::EXTERN}
};

std::string TokenInfo::toString(TokenType type) {
    switch (type) {
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INT: return "int";
        case TokenType::FLOAT: return "float";
        case TokenType::STRING_KW: return "string";
        case TokenType::BOOL: return "bool";
        case TokenType::VOID: return "void";
        case TokenType::IF: return "if";
        case TokenType::ELSE: return "else";
        case TokenType::WHILE: return "while";
        case TokenType::FOR: return "for";
        case TokenType::RETURN: return "return";
        case TokenType::TRUE: return "true";
        case TokenType::FALSE: return "false";
        case TokenType::IMPORT: return "import";
        case TokenType::EXTERN: return "extern";
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::MULTIPLY: return "*";
        case TokenType::DIVIDE: return "/";
        case TokenType::MODULO: return "%";
        case TokenType::ASSIGN: return "=";
        case TokenType::EQUAL: return "==";
        case TokenType::NOT_EQUAL: return "!=";
        case TokenType::LESS: return "<";
        case TokenType::LESS_EQUAL: return "<=";
        case TokenType::GREATER: return ">";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::AND: return "&&";
        case TokenType::OR: return "||";
        case TokenType::NOT: return "!";
        case TokenType::LEFT_PAREN: return "(";
        case TokenType::RIGHT_PAREN: return ")";
        case TokenType::LEFT_BRACE: return "{";
        case TokenType::RIGHT_BRACE: return "}";
        case TokenType::LEFT_BRACKET: return "[";
        case TokenType::RIGHT_BRACKET: return "]";
        case TokenType::SEMICOLON: return ";";
        case TokenType::COMMA: return ",";
        case TokenType::DOT: return ".";
        case TokenType::COLON: return ":";
        case TokenType::NAMESPACE_SEP: return "::";
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}

bool TokenInfo::isKeyword(const std::string& text) {
    return keywords.find(text) != keywords.end();
}

TokenType TokenInfo::getKeywordType(const std::string& text) {
    auto it = keywords.find(text);
    return (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
}

} // namespace Thor