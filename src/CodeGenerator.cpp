#include "CodeGenerator.h"
#include <iostream>

namespace Thor {

CodeGenerator::CodeGenerator() : indentLevel(0) {}

void CodeGenerator::reset() {
    output.str("");
    output.clear();
    includedHeaders.clear();
    builtinFunctions.clear();
    indentLevel = 0;
}

void CodeGenerator::indent() {
    for (int i = 0; i < indentLevel; ++i) {
        output << "    ";
    }
}

void CodeGenerator::generateIncludes() {
    output << "#include <stdio.h>\n";
    output << "#include <stdlib.h>\n";
    output << "#include <string.h>\n";
    output << "\n";
}

void CodeGenerator::generateBuiltinFunctions() {
    if (builtinFunctions.count("std::println")) {
        output << "// Built-in function for std::println\n";
        output << "void thor_println(const char* str) {\n";
        output << "    printf(\"%s\\n\", str);\n";
        output << "}\n\n";
    }
    
    if (builtinFunctions.count("std::print")) {
        output << "// Built-in function for std::print\n";
        output << "void thor_print(const char* str) {\n";
        output << "    printf(\"%s\", str);\n";
        output << "}\n\n";
    }
}

std::string CodeGenerator::generate(const Program* program) {
    reset();
    
    // First pass: collect builtin function usage by scanning the AST
    for (const auto& stmt : program->statements) {
        scanForBuiltins(stmt.get());
    }
    
    generateIncludes();
    generateBuiltinFunctions();
    
    // Generate all statements
    for (const auto& stmt : program->statements) {
        generateStatement(stmt.get());
        output << "\n";
    }
    
    return output.str();
}

void CodeGenerator::generateExpression(const Expression* expr) {
    if (auto numLit = dynamic_cast<const NumberLiteral*>(expr)) {
        generateNumberLiteral(numLit);
    } else if (auto strLit = dynamic_cast<const StringLiteral*>(expr)) {
        generateStringLiteral(strLit);
    } else if (auto boolLit = dynamic_cast<const BoolLiteral*>(expr)) {
        generateBoolLiteral(boolLit);
    } else if (auto id = dynamic_cast<const Identifier*>(expr)) {
        generateIdentifier(id);
    } else if (auto binOp = dynamic_cast<const BinaryOperation*>(expr)) {
        generateBinaryOperation(binOp);
    } else if (auto unOp = dynamic_cast<const UnaryOperation*>(expr)) {
        generateUnaryOperation(unOp);
    } else if (auto call = dynamic_cast<const FunctionCall*>(expr)) {
        generateFunctionCall(call);
    }
}

void CodeGenerator::generateStatement(const Statement* stmt) {
    if (auto varDecl = dynamic_cast<const VariableDeclaration*>(stmt)) {
        generateVariableDeclaration(varDecl);
    } else if (auto assign = dynamic_cast<const Assignment*>(stmt)) {
        generateAssignment(assign);
    } else if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt)) {
        generateExpressionStatement(exprStmt);
    } else if (auto retStmt = dynamic_cast<const ReturnStatement*>(stmt)) {
        generateReturnStatement(retStmt);
    } else if (auto block = dynamic_cast<const Block*>(stmt)) {
        generateBlock(block);
    } else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt)) {
        generateIfStatement(ifStmt);
    } else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt)) {
        generateWhileStatement(whileStmt);
    } else if (auto funcDecl = dynamic_cast<const FunctionDeclaration*>(stmt)) {
        generateFunctionDeclaration(funcDecl);
    } else if (auto externDecl = dynamic_cast<const ExternDeclaration*>(stmt)) {
        generateExternDeclaration(externDecl);
    } else if (auto importStmt = dynamic_cast<const ImportStatement*>(stmt)) {
        // Import statements should have been processed by ImportProcessor
        // This is just for safety - they should not appear here
        indent();
        output << "// Import: " << importStmt->moduleName;
    }
}

void CodeGenerator::generateNumberLiteral(const NumberLiteral* lit) {
    output << lit->value;
}

void CodeGenerator::generateStringLiteral(const StringLiteral* lit) {
    output << "\"" << lit->value << "\"";
}

void CodeGenerator::generateBoolLiteral(const BoolLiteral* lit) {
    output << (lit->value ? "1" : "0");
}

void CodeGenerator::generateIdentifier(const Identifier* id) {
    output << id->name;
}

void CodeGenerator::generateBinaryOperation(const BinaryOperation* op) {
    output << "(";
    generateExpression(op->left.get());
    output << " " << op->operator_ << " ";
    generateExpression(op->right.get());
    output << ")";
}

void CodeGenerator::generateUnaryOperation(const UnaryOperation* op) {
    output << op->operator_;
    output << "(";
    generateExpression(op->operand.get());
    output << ")";
}

void CodeGenerator::generateFunctionCall(const FunctionCall* call) {
    if (isBuiltinFunction(call->name)) {
        addBuiltinFunction(call->name);
        output << translateBuiltinFunction(call->name);
    } else {
        output << call->name;
    }
    
    output << "(";
    for (size_t i = 0; i < call->arguments.size(); ++i) {
        if (i > 0) output << ", ";
        generateExpression(call->arguments[i].get());
    }
    output << ")";
}

void CodeGenerator::generateVariableDeclaration(const VariableDeclaration* decl) {
    indent();
    output << typeToCType(decl->type) << " " << decl->name;
    
    if (decl->initializer) {
        output << " = ";
        generateExpression(decl->initializer.get());
    }
    
    output << ";";
}

void CodeGenerator::generateAssignment(const Assignment* assign) {
    indent();
    output << assign->name << " = ";
    generateExpression(assign->value.get());
    output << ";";
}

void CodeGenerator::generateExpressionStatement(const ExpressionStatement* stmt) {
    indent();
    generateExpression(stmt->expression.get());
    output << ";";
}

void CodeGenerator::generateReturnStatement(const ReturnStatement* stmt) {
    indent();
    output << "return";
    if (stmt->value) {
        output << " ";
        generateExpression(stmt->value.get());
    }
    output << ";";
}

void CodeGenerator::generateBlock(const Block* block, bool addBraces) {
    if (addBraces) {
        indent();
        output << "{\n";
        indentLevel++;
    }
    
    for (const auto& stmt : block->statements) {
        generateStatement(stmt.get());
        output << "\n";
    }
    
    if (addBraces) {
        indentLevel--;
        indent();
        output << "}";
    }
}

void CodeGenerator::generateIfStatement(const IfStatement* stmt) {
    indent();
    output << "if (";
    generateExpression(stmt->condition.get());
    output << ") ";
    
    if (auto block = dynamic_cast<const Block*>(stmt->thenStatement.get())) {
        output << "{\n";
        indentLevel++;
        generateBlock(block, false);
        indentLevel--;
        indent();
        output << "}";
    } else {
        output << "\n";
        indentLevel++;
        generateStatement(stmt->thenStatement.get());
        indentLevel--;
    }
    
    if (stmt->elseStatement) {
        output << " else ";
        
        if (auto block = dynamic_cast<const Block*>(stmt->elseStatement.get())) {
            output << "{\n";
            indentLevel++;
            generateBlock(block, false);
            indentLevel--;
            indent();
            output << "}";
        } else {
            output << "\n";
            indentLevel++;
            generateStatement(stmt->elseStatement.get());
            indentLevel--;
        }
    }
}

void CodeGenerator::generateWhileStatement(const WhileStatement* stmt) {
    indent();
    output << "while (";
    generateExpression(stmt->condition.get());
    output << ") ";
    
    if (auto block = dynamic_cast<const Block*>(stmt->body.get())) {
        output << "{\n";
        indentLevel++;
        generateBlock(block, false);
        indentLevel--;
        indent();
        output << "}";
    } else {
        output << "\n";
        indentLevel++;
        generateStatement(stmt->body.get());
        indentLevel--;
    }
}

void CodeGenerator::generateFunctionDeclaration(const FunctionDeclaration* decl) {
    output << typeToCType(decl->returnType) << " " << decl->name << "(";
    
    for (size_t i = 0; i < decl->parameters.size(); ++i) {
        if (i > 0) output << ", ";
        output << typeToCType(decl->parameters[i].type) << " " << decl->parameters[i].name;
    }
    
    output << ") ";
    
    if (decl->body) {
        output << "{\n";
        indentLevel++;
        generateBlock(decl->body.get(), false);
        indentLevel--;
        output << "}";
    } else {
        output << ";";
    }
}

void CodeGenerator::generateExternDeclaration(const ExternDeclaration* decl) {
    output << "extern " << typeToCType(decl->returnType) << " " << decl->name << "(";
    
    for (size_t i = 0; i < decl->parameters.size(); ++i) {
        if (i > 0) output << ", ";
        output << typeToCType(decl->parameters[i].type) << " " << decl->parameters[i].name;
    }
    
    output << ");";
}

bool CodeGenerator::isBuiltinFunction(const std::string& name) {
    return name == "std::println" || name == "std::print" || name == "println" || name == "print";
}

void CodeGenerator::addBuiltinFunction(const std::string& name) {
    if (name == "println" || name == "std::println") {
        builtinFunctions.insert("std::println");
    } else if (name == "print" || name == "std::print") {
        builtinFunctions.insert("std::print");
    }
}

std::string CodeGenerator::translateBuiltinFunction(const std::string& name) {
    if (name == "std::println" || name == "println") return "thor_println";
    if (name == "std::print" || name == "print") return "thor_print";
    return name;
}

void CodeGenerator::scanForBuiltins(const ASTNode* node) {
    if (auto call = dynamic_cast<const FunctionCall*>(node)) {
        if (isBuiltinFunction(call->name)) {
            addBuiltinFunction(call->name);
        }
        // Scan arguments
        for (const auto& arg : call->arguments) {
            scanForBuiltins(arg.get());
        }
    } else if (auto binOp = dynamic_cast<const BinaryOperation*>(node)) {
        scanForBuiltins(binOp->left.get());
        scanForBuiltins(binOp->right.get());
    } else if (auto unOp = dynamic_cast<const UnaryOperation*>(node)) {
        scanForBuiltins(unOp->operand.get());
    } else if (auto varDecl = dynamic_cast<const VariableDeclaration*>(node)) {
        if (varDecl->initializer) {
            scanForBuiltins(varDecl->initializer.get());
        }
    } else if (auto assign = dynamic_cast<const Assignment*>(node)) {
        scanForBuiltins(assign->value.get());
    } else if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(node)) {
        scanForBuiltins(exprStmt->expression.get());
    } else if (auto retStmt = dynamic_cast<const ReturnStatement*>(node)) {
        if (retStmt->value) {
            scanForBuiltins(retStmt->value.get());
        }
    } else if (auto block = dynamic_cast<const Block*>(node)) {
        for (const auto& stmt : block->statements) {
            scanForBuiltins(stmt.get());
        }
    } else if (auto ifStmt = dynamic_cast<const IfStatement*>(node)) {
        scanForBuiltins(ifStmt->condition.get());
        scanForBuiltins(ifStmt->thenStatement.get());
        if (ifStmt->elseStatement) {
            scanForBuiltins(ifStmt->elseStatement.get());
        }
    } else if (auto whileStmt = dynamic_cast<const WhileStatement*>(node)) {
        scanForBuiltins(whileStmt->condition.get());
        scanForBuiltins(whileStmt->body.get());
    } else if (auto funcDecl = dynamic_cast<const FunctionDeclaration*>(node)) {
        if (funcDecl->body) {
            scanForBuiltins(funcDecl->body.get());
        }
    } else if (auto externDecl = dynamic_cast<const ExternDeclaration*>(node)) {
        // Extern declarations don't contain builtins, just skip
    } else if (auto importStmt = dynamic_cast<const ImportStatement*>(node)) {
        // Import statements should have been processed already
        // No need to scan them for builtins
    }
}

} // namespace Thor