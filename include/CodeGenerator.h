#pragma once
#include "AST.h"
#include <string>
#include <sstream>
#include <unordered_set>

namespace Thor {

class CodeGenerator {
private:
    std::ostringstream output;
    std::unordered_set<std::string> includedHeaders;
    std::unordered_set<std::string> builtinFunctions;
    int indentLevel;
    
    void indent();
    void generateIncludes();
    void generateBuiltinFunctions();
    
    void generateExpression(const Expression* expr);
    void generateStatement(const Statement* stmt);
    
    void generateNumberLiteral(const NumberLiteral* lit);
    void generateStringLiteral(const StringLiteral* lit);
    void generateBoolLiteral(const BoolLiteral* lit);
    void generateIdentifier(const Identifier* id);
    void generateBinaryOperation(const BinaryOperation* op);
    void generateUnaryOperation(const UnaryOperation* op);
    void generateFunctionCall(const FunctionCall* call);
    
    void generateVariableDeclaration(const VariableDeclaration* decl);
    void generateAssignment(const Assignment* assign);
    void generateExpressionStatement(const ExpressionStatement* stmt);
    void generateReturnStatement(const ReturnStatement* stmt);
    void generateBlock(const Block* block, bool addBraces = true);
    void generateIfStatement(const IfStatement* stmt);
    void generateWhileStatement(const WhileStatement* stmt);
    void generateFunctionDeclaration(const FunctionDeclaration* decl);
    void generateExternDeclaration(const ExternDeclaration* decl);
    
    bool isBuiltinFunction(const std::string& name);
    void addBuiltinFunction(const std::string& name);
    std::string translateBuiltinFunction(const std::string& name);
    void scanForBuiltins(const ASTNode* node);
    
public:
    CodeGenerator();
    std::string generate(const Program* program);
    void reset();
};

} // namespace Thor