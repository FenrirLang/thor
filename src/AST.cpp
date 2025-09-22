#include "AST.h"
#include <sstream>

namespace Thor {

std::string typeToString(Type type) {
    switch (type) {
        case Type::INT: return "int";
        case Type::FLOAT: return "float";
        case Type::STRING: return "string";
        case Type::BOOL: return "bool";
        case Type::VOID: return "void";
        case Type::VOID_PTR: return "void*";
        case Type::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

std::string typeToCType(Type type) {
    switch (type) {
        case Type::INT: return "int";
        case Type::FLOAT: return "float";
        case Type::STRING: return "char*";
        case Type::BOOL: return "int";
        case Type::VOID: return "void";
        case Type::VOID_PTR: return "void*";
        case Type::UNKNOWN: return "void";
        default: return "void";
    }
}

std::string ASTNode::getIndent(int level) const {
    return std::string(level * 2, ' ');
}

// NumberLiteral implementation
NumberLiteral::NumberLiteral(const std::string& val) : value(val) {
    // Determine type based on whether value contains a decimal point
    type = (value.find('.') != std::string::npos) ? Type::FLOAT : Type::INT;
}

std::string NumberLiteral::toString(int indent) const {
    return getIndent(indent) + "NumberLiteral: " + value + " (" + typeToString(type) + ")";
}

// StringLiteral implementation
StringLiteral::StringLiteral(const std::string& val) : value(val) {
    type = Type::STRING;
}

std::string StringLiteral::toString(int indent) const {
    return getIndent(indent) + "StringLiteral: \"" + value + "\"";
}

// BoolLiteral implementation
BoolLiteral::BoolLiteral(bool val) : value(val) {
    type = Type::BOOL;
}

std::string BoolLiteral::toString(int indent) const {
    return getIndent(indent) + "BoolLiteral: " + (value ? "true" : "false");
}

// Identifier implementation
Identifier::Identifier(const std::string& n) : name(n) {}

std::string Identifier::toString(int indent) const {
    return getIndent(indent) + "Identifier: " + name;
}

// BinaryOperation implementation
BinaryOperation::BinaryOperation(std::unique_ptr<Expression> l, const std::string& op, std::unique_ptr<Expression> r)
    : left(std::move(l)), operator_(op), right(std::move(r)) {}

std::string BinaryOperation::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "BinaryOperation: " << operator_ << "\n";
    oss << left->toString(indent + 1) << "\n";
    oss << right->toString(indent + 1);
    return oss.str();
}

// UnaryOperation implementation
UnaryOperation::UnaryOperation(const std::string& op, std::unique_ptr<Expression> expr)
    : operator_(op), operand(std::move(expr)) {}

std::string UnaryOperation::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "UnaryOperation: " << operator_ << "\n";
    oss << operand->toString(indent + 1);
    return oss.str();
}

// FunctionCall implementation
FunctionCall::FunctionCall(const std::string& n) : name(n) {}

void FunctionCall::addArgument(std::unique_ptr<Expression> arg) {
    arguments.push_back(std::move(arg));
}

std::string FunctionCall::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "FunctionCall: " << name << "\n";
    for (const auto& arg : arguments) {
        oss << arg->toString(indent + 1) << "\n";
    }
    return oss.str();
}

// VariableDeclaration implementation
VariableDeclaration::VariableDeclaration(Type t, const std::string& n, std::unique_ptr<Expression> init)
    : type(t), name(n), initializer(std::move(init)) {}

std::string VariableDeclaration::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "VariableDeclaration: " << typeToString(type) << " " << name;
    if (initializer) {
        oss << "\n" << initializer->toString(indent + 1);
    }
    return oss.str();
}

// Assignment implementation
Assignment::Assignment(const std::string& n, std::unique_ptr<Expression> val)
    : name(n), value(std::move(val)) {}

std::string Assignment::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "Assignment: " << name << "\n";
    oss << value->toString(indent + 1);
    return oss.str();
}

// ExpressionStatement implementation
ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expr)
    : expression(std::move(expr)) {}

std::string ExpressionStatement::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "ExpressionStatement:\n";
    oss << expression->toString(indent + 1);
    return oss.str();
}

// ReturnStatement implementation
ReturnStatement::ReturnStatement(std::unique_ptr<Expression> val) : value(std::move(val)) {}

std::string ReturnStatement::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "ReturnStatement";
    if (value) {
        oss << ":\n" << value->toString(indent + 1);
    }
    return oss.str();
}

// ImportStatement implementation
ImportStatement::ImportStatement(const std::string& module) : moduleName(module) {}

std::string ImportStatement::toString(int indent) const {
    return getIndent(indent) + "ImportStatement: " + moduleName;
}

// ExternDeclaration implementation
ExternDeclaration::ExternDeclaration(Type ret, const std::string& n)
    : returnType(ret), name(n) {}

void ExternDeclaration::addParameter(Type type, const std::string& name) {
    parameters.emplace_back(type, name);
}

std::string ExternDeclaration::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "ExternDeclaration: " << typeToCType(returnType) << " " << name << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << typeToCType(parameters[i].type) << " " << parameters[i].name;
    }
    oss << ");";
    return oss.str();
}

// Block implementation
void Block::addStatement(std::unique_ptr<Statement> stmt) {
    statements.push_back(std::move(stmt));
}

std::string Block::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "Block:\n";
    for (const auto& stmt : statements) {
        oss << stmt->toString(indent + 1) << "\n";
    }
    return oss.str();
}

// IfStatement implementation
IfStatement::IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> then, std::unique_ptr<Statement> els)
    : condition(std::move(cond)), thenStatement(std::move(then)), elseStatement(std::move(els)) {}

std::string IfStatement::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "IfStatement:\n";
    oss << getIndent(indent + 1) << "Condition:\n";
    oss << condition->toString(indent + 2) << "\n";
    oss << getIndent(indent + 1) << "Then:\n";
    oss << thenStatement->toString(indent + 2);
    if (elseStatement) {
        oss << "\n" << getIndent(indent + 1) << "Else:\n";
        oss << elseStatement->toString(indent + 2);
    }
    return oss.str();
}

// WhileStatement implementation
WhileStatement::WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> b)
    : condition(std::move(cond)), body(std::move(b)) {}

std::string WhileStatement::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "WhileStatement:\n";
    oss << getIndent(indent + 1) << "Condition:\n";
    oss << condition->toString(indent + 2) << "\n";
    oss << getIndent(indent + 1) << "Body:\n";
    oss << body->toString(indent + 2);
    return oss.str();
}

// Parameter implementation
Parameter::Parameter(Type t, const std::string& n) : type(t), name(n) {}

// FunctionDeclaration implementation
FunctionDeclaration::FunctionDeclaration(Type ret, const std::string& n)
    : returnType(ret), name(n) {}

void FunctionDeclaration::addParameter(Type type, const std::string& name) {
    parameters.emplace_back(type, name);
}

void FunctionDeclaration::setBody(std::unique_ptr<Block> b) {
    body = std::move(b);
}

std::string FunctionDeclaration::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "FunctionDeclaration: " << typeToString(returnType) << " " << name << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << typeToString(parameters[i].type) << " " << parameters[i].name;
    }
    oss << ")\n";
    if (body) {
        oss << body->toString(indent + 1);
    }
    return oss.str();
}

// Program implementation
void Program::addStatement(std::unique_ptr<Statement> stmt) {
    statements.push_back(std::move(stmt));
}

std::string Program::toString(int indent) const {
    std::ostringstream oss;
    oss << getIndent(indent) << "Program:\n";
    for (const auto& stmt : statements) {
        oss << stmt->toString(indent + 1) << "\n";
    }
    return oss.str();
}

} // namespace Thor