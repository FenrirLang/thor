#pragma once
#include <vector>
#include <memory>
#include <string>

namespace Thor {

// Forward declarations
class ASTNode;
class Expression;
class Statement;

// Basic types in Thor
enum class Type {
    INT,
    FLOAT,
    STRING,
    BOOL,
    VOID,
    VOID_PTR,  // void* type for external function pointers
    UNKNOWN
};

std::string typeToString(Type type);
std::string typeToCType(Type type);

// Base AST Node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string toString(int indent = 0) const = 0;
protected:
    std::string getIndent(int level) const;
};

// Expression base class
class Expression : public ASTNode {
public:
    Type type = Type::UNKNOWN;
};

// Statement base class  
class Statement : public ASTNode {
};

// Literal expressions
class NumberLiteral : public Expression {
public:
    std::string value;
    
    NumberLiteral(const std::string& val);
    std::string toString(int indent = 0) const override;
};

class StringLiteral : public Expression {
public:
    std::string value;
    
    StringLiteral(const std::string& val);
    std::string toString(int indent = 0) const override;
};

class BoolLiteral : public Expression {
public:
    bool value;
    
    BoolLiteral(bool val);
    std::string toString(int indent = 0) const override;
};

class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& n);
    std::string toString(int indent = 0) const override;
};

// Binary operations
class BinaryOperation : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::string operator_;
    std::unique_ptr<Expression> right;
    
    BinaryOperation(std::unique_ptr<Expression> l, const std::string& op, std::unique_ptr<Expression> r);
    std::string toString(int indent = 0) const override;
};

// Unary operations
class UnaryOperation : public Expression {
public:
    std::string operator_;
    std::unique_ptr<Expression> operand;
    
    UnaryOperation(const std::string& op, std::unique_ptr<Expression> expr);
    std::string toString(int indent = 0) const override;
};

// Function calls
class FunctionCall : public Expression {
public:
    std::string name;
    std::vector<std::unique_ptr<Expression>> arguments;
    
    FunctionCall(const std::string& n);
    void addArgument(std::unique_ptr<Expression> arg);
    std::string toString(int indent = 0) const override;
};

// Variable declarations
class VariableDeclaration : public Statement {
public:
    Type type;
    std::string name;
    std::unique_ptr<Expression> initializer;
    
    VariableDeclaration(Type t, const std::string& n, std::unique_ptr<Expression> init = nullptr);
    std::string toString(int indent = 0) const override;
};

// Assignment statements
class Assignment : public Statement {
public:
    std::string name;
    std::unique_ptr<Expression> value;
    
    Assignment(const std::string& n, std::unique_ptr<Expression> val);
    std::string toString(int indent = 0) const override;
};

// Expression statements
class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;
    
    ExpressionStatement(std::unique_ptr<Expression> expr);
    std::string toString(int indent = 0) const override;
};

// Return statements
class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> value;
    
    ReturnStatement(std::unique_ptr<Expression> val = nullptr);
    std::string toString(int indent = 0) const override;
};

// Import statements
class ImportStatement : public Statement {
public:
    std::string moduleName;
    
    ImportStatement(const std::string& module);
    std::string toString(int indent = 0) const override;
};

// Block statements
class Block : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    
    void addStatement(std::unique_ptr<Statement> stmt);
    std::string toString(int indent = 0) const override;
};

// If statements
class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenStatement;
    std::unique_ptr<Statement> elseStatement;
    
    IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> then, std::unique_ptr<Statement> els = nullptr);
    std::string toString(int indent = 0) const override;
};

// While statements
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
    
    WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> b);
    std::string toString(int indent = 0) const override;
};

// Function parameters
class Parameter {
public:
    Type type;
    std::string name;
    
    Parameter(Type t, const std::string& n);
};

// Function declarations
class FunctionDeclaration : public Statement {
public:
    Type returnType;
    std::string name;
    std::vector<Parameter> parameters;
    std::unique_ptr<Block> body;
    
    FunctionDeclaration(Type ret, const std::string& n);
    void addParameter(Type type, const std::string& name);
    void setBody(std::unique_ptr<Block> b);
    std::string toString(int indent = 0) const override;
};

// Extern function declarations
class ExternDeclaration : public Statement {
public:
    Type returnType;
    std::string name;
    std::vector<Parameter> parameters;
    
    ExternDeclaration(Type ret, const std::string& n);
    void addParameter(Type type, const std::string& name);
    std::string toString(int indent = 0) const override;
};

// Root program node
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    
    void addStatement(std::unique_ptr<Statement> stmt);
    std::string toString(int indent = 0) const override;
};

} // namespace Thor