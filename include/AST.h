#pragma once
#include <memory>
#include <vector>
#include <string>

// Forward declarations
struct Type;

// Base AST Node
struct ASTNode {
    virtual ~ASTNode() = default;
};

// Expression nodes
struct Expression : ASTNode {
    std::shared_ptr<Type> type;
};

struct LiteralExpression : Expression {
    std::string value;
    enum LiteralType { INTEGER, STRING, BOOLEAN } literalType;
    
    LiteralExpression(const std::string& val, LiteralType lt) 
        : value(val), literalType(lt) {}
};

struct IdentifierExpression : Expression {
    std::string name;
    
    IdentifierExpression(const std::string& n) : name(n) {}
};

struct BinaryExpression : Expression {
    std::shared_ptr<Expression> left;
    std::string operator_;
    std::shared_ptr<Expression> right;
    
    BinaryExpression(std::shared_ptr<Expression> l, const std::string& op, std::shared_ptr<Expression> r)
        : left(l), operator_(op), right(r) {}
};

struct UnaryExpression : Expression {
    std::string operator_;
    std::shared_ptr<Expression> operand;
    
    UnaryExpression(const std::string& op, std::shared_ptr<Expression> expr)
        : operator_(op), operand(expr) {}
};

struct CallExpression : Expression {
    std::shared_ptr<Expression> callee;
    std::vector<std::shared_ptr<Expression>> arguments;
    
    CallExpression(std::shared_ptr<Expression> c, std::vector<std::shared_ptr<Expression>> args)
        : callee(c), arguments(args) {}
};

struct MemberExpression : Expression {
    std::shared_ptr<Expression> object;
    std::string property;
    
    MemberExpression(std::shared_ptr<Expression> obj, const std::string& prop)
        : object(obj), property(prop) {}
};

struct ArrayExpression : Expression {
    std::vector<std::shared_ptr<Expression>> elements;
    
    ArrayExpression(std::vector<std::shared_ptr<Expression>> elems)
        : elements(elems) {}
};

struct FormatStringExpression : Expression {
    std::string format;
    std::vector<std::shared_ptr<Expression>> arguments;
    
    FormatStringExpression(const std::string& fmt, std::vector<std::shared_ptr<Expression>> args)
        : format(fmt), arguments(args) {}
};

// Statement nodes
struct Statement : ASTNode {};

struct ExpressionStatement : Statement {
    std::shared_ptr<Expression> expression;
    
    ExpressionStatement(std::shared_ptr<Expression> expr) : expression(expr) {}
};

struct VariableDeclaration : Statement {
    std::string name;
    std::shared_ptr<Type> type;
    std::shared_ptr<Expression> initializer;
    
    VariableDeclaration(const std::string& n, std::shared_ptr<Type> t, std::shared_ptr<Expression> init = nullptr)
        : name(n), type(t), initializer(init) {}
};

struct BlockStatement : Statement {
    std::vector<std::shared_ptr<Statement>> statements;
    
    BlockStatement(std::vector<std::shared_ptr<Statement>> stmts) : statements(stmts) {}
};

struct IfStatement : Statement {
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> thenBranch;
    std::shared_ptr<Statement> elseBranch;
    
    IfStatement(std::shared_ptr<Expression> cond, std::shared_ptr<Statement> then, std::shared_ptr<Statement> els = nullptr)
        : condition(cond), thenBranch(then), elseBranch(els) {}
};

struct WhileStatement : Statement {
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
    
    WhileStatement(std::shared_ptr<Expression> cond, std::shared_ptr<Statement> b)
        : condition(cond), body(b) {}
};

struct ReturnStatement : Statement {
    std::shared_ptr<Expression> value;
    
    ReturnStatement(std::shared_ptr<Expression> val = nullptr) : value(val) {}
};

// Type system
struct Type {
    enum TypeKind { VOID_TYPE, INTEGER_TYPE, STRING_TYPE, BOOLEAN_TYPE, ARRAY_TYPE, FUNCTION_TYPE } kind;
    std::shared_ptr<Type> elementType; // For arrays
    std::vector<std::shared_ptr<Type>> parameterTypes; // For functions
    std::shared_ptr<Type> returnType; // For functions
    
    Type(TypeKind k) : kind(k) {}
    
    static std::shared_ptr<Type> createVoid() { return std::make_shared<Type>(VOID_TYPE); }
    static std::shared_ptr<Type> createInt() { return std::make_shared<Type>(INTEGER_TYPE); }
    static std::shared_ptr<Type> createString() { return std::make_shared<Type>(STRING_TYPE); }
    static std::shared_ptr<Type> createBoolean() { return std::make_shared<Type>(BOOLEAN_TYPE); }
    static std::shared_ptr<Type> createArray(std::shared_ptr<Type> elem) {
        auto type = std::make_shared<Type>(ARRAY_TYPE);
        type->elementType = elem;
        return type;
    }
};

struct Parameter {
    std::string name;
    std::shared_ptr<Type> type;
    
    Parameter(const std::string& n, std::shared_ptr<Type> t) : name(n), type(t) {}
};

struct FunctionDeclaration : Statement {
    std::string name;
    std::vector<Parameter> parameters;
    std::shared_ptr<Type> returnType;
    std::shared_ptr<BlockStatement> body;
    
    FunctionDeclaration(const std::string& n, std::vector<Parameter> params, 
                       std::shared_ptr<Type> ret, std::shared_ptr<BlockStatement> b)
        : name(n), parameters(params), returnType(ret), body(b) {}
};

struct PackageDeclaration : Statement {
    std::string name;
    
    PackageDeclaration(const std::string& n) : name(n) {}
};

struct ImportDeclaration : Statement {
    std::string module;
    
    ImportDeclaration(const std::string& m) : module(m) {}
};

// Root node
struct Program : ASTNode {
    std::shared_ptr<PackageDeclaration> package;
    std::vector<std::shared_ptr<ImportDeclaration>> imports;
    std::vector<std::shared_ptr<Statement>> statements;
    
    Program() = default;
};