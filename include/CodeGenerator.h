#pragma once
#include "AST.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

class CodeGenerator {
private:
    std::ostringstream output;
    int indentLevel;
    std::unordered_map<std::string, std::shared_ptr<Program>> modules;
    std::unordered_map<std::string, std::string> builtinFunctions;
    std::shared_ptr<Program> currentProgram; // Track current program being generated
    
    void indent();
    void writeLine(const std::string& line = "");
    void write(const std::string& text);
    
    // Generation methods
    void generateIncludes();
    void generateBuiltinFunctions();
    void generateType(std::shared_ptr<Type> type);
    void generateExpression(std::shared_ptr<Expression> expr);
    void generateStatement(std::shared_ptr<Statement> stmt);
    void generateFunction(std::shared_ptr<FunctionDeclaration> func);
    void generateProgram(std::shared_ptr<Program> program);
    
    // Helper methods
    std::string getTypeName(std::shared_ptr<Type> type);
    std::string getCTypeName(std::shared_ptr<Type> type);
    std::string generateFormatString(const std::string& format, 
                                   const std::vector<std::shared_ptr<Expression>>& args);
    void initializeBuiltinFunctions();
    
public:
    CodeGenerator();
    std::string generate(std::shared_ptr<Program> program, 
                        const std::unordered_map<std::string, std::shared_ptr<Program>>& importedModules);
};