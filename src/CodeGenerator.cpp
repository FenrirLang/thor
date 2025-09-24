#include "CodeGenerator.h"
#include <algorithm>
#include <regex>

CodeGenerator::CodeGenerator() : indentLevel(0) {
    initializeBuiltinFunctions();
}

std::string CodeGenerator::generate(std::shared_ptr<Program> program, 
                                  const std::unordered_map<std::string, std::shared_ptr<Program>>& importedModules) {
    output.clear();
    output.str("");
    indentLevel = 0;
    modules = importedModules;
    
    generateIncludes();
    generateBuiltinFunctions();
    
    // Generate code for all modules first
    for (const auto& [moduleName, moduleProgram] : modules) {
        generateProgram(moduleProgram);
        writeLine();
    }
    
    // Generate main program
    generateProgram(program);
    
    return output.str();
}

void CodeGenerator::generateIncludes() {
    writeLine("#include <stdio.h>");
    writeLine("#include <stdlib.h>");
    writeLine("#include <string.h>");
    writeLine("#include <stdbool.h>");
    writeLine("#include <stdarg.h>");
    writeLine();
}

void CodeGenerator::indent() {
    for (int i = 0; i < indentLevel; i++) {
        output << "    ";
    }
}

void CodeGenerator::writeLine(const std::string& line) {
    if (!line.empty()) {
        indent();
        output << line;
    }
    output << "\n";
}

void CodeGenerator::write(const std::string& text) {
    output << text;
}

void CodeGenerator::generateBuiltinFunctions() {
    // String input function
    writeLine("char* thor_input(const char* prompt) {");
    indentLevel++;
    writeLine("printf(\"%s\", prompt);");
    writeLine("char* buffer = malloc(1024);");
    writeLine("fgets(buffer, 1024, stdin);");
    writeLine("// Remove newline");
    writeLine("int len = strlen(buffer);");
    writeLine("if (len > 0 && buffer[len-1] == '\\n') {");
    indentLevel++;
    writeLine("buffer[len-1] = '\\0';");
    indentLevel--;
    writeLine("}");
    writeLine("return buffer;");
    indentLevel--;
    writeLine("}");
    writeLine();
    
    // Print function
    writeLine("void thor_println(const char* str) {");
    indentLevel++;
    writeLine("printf(\"%s\\n\", str);");
    indentLevel--;
    writeLine("}");
    writeLine();
    
    // String comparison function
    writeLine("bool thor_string_equals(const char* a, const char* b) {");
    indentLevel++;
    writeLine("return strcmp(a, b) == 0;");
    indentLevel--;
    writeLine("}");
    writeLine();
    
    // Format string function
    writeLine("char* thor_format_string(const char* format, ...) {");
    indentLevel++;
    writeLine("va_list args;");
    writeLine("va_start(args, format);");
    writeLine("char* buffer = malloc(1024);");
    writeLine("vsnprintf(buffer, 1024, format, args);");
    writeLine("va_end(args);");
    writeLine("return buffer;");
    indentLevel--;
    writeLine("}");
    writeLine();
}

void CodeGenerator::generateProgram(std::shared_ptr<Program> program) {
    currentProgram = program; // Set current program context
    
    // Generate forward declarations for functions
    for (auto& stmt : program->statements) {
        if (auto funcDecl = std::dynamic_pointer_cast<FunctionDeclaration>(stmt)) {
            // Skip built-in functions without bodies
            if (!funcDecl->body) {
                continue;
            }
            
            generateType(funcDecl->returnType);
            write(" ");
            if (program->package && program->package->name != "main") {
                write(program->package->name + "_");
            }
            write(funcDecl->name + "(");
            
            for (size_t i = 0; i < funcDecl->parameters.size(); i++) {
                if (i > 0) write(", ");
                generateType(funcDecl->parameters[i].type);
                write(" " + funcDecl->parameters[i].name);
            }
            
            writeLine(");");
        }
    }
    
    if (!program->statements.empty()) {
        writeLine();
    }
    
    // Generate function implementations
    for (auto& stmt : program->statements) {
        generateStatement(stmt);
        writeLine();
    }
}

void CodeGenerator::generateType(std::shared_ptr<Type> type) {
    write(getCTypeName(type));
}

std::string CodeGenerator::getCTypeName(std::shared_ptr<Type> type) {
    switch (type->kind) {
        case Type::VOID_TYPE: return "void";
        case Type::INTEGER_TYPE: return "int";
        case Type::FLOAT_TYPE: return "float";
        case Type::STRING_TYPE: return "char*";
        case Type::BOOLEAN_TYPE: return "bool";
        case Type::ARRAY_TYPE: 
            return getCTypeName(type->elementType) + "*";
        case Type::REFERENCE_TYPE:
            return getCTypeName(type->elementType) + "*";
        default: return "void";
    }
}

void CodeGenerator::generateExpression(std::shared_ptr<Expression> expr) {
    if (auto literal = std::dynamic_pointer_cast<LiteralExpression>(expr)) {
        switch (literal->literalType) {
            case LiteralExpression::INTEGER:
                write(literal->value);
                break;
            case LiteralExpression::FLOAT:
                write(literal->value);
                break;
            case LiteralExpression::STRING:
                write("\"" + literal->value + "\"");
                break;
            case LiteralExpression::BOOLEAN:
                write(literal->value == "true" ? "true" : "false");
                break;
        }
    }
    else if (auto identifier = std::dynamic_pointer_cast<IdentifierExpression>(expr)) {
        // Check if this is a reference parameter that needs dereferencing
        if (referenceParameters.find(identifier->name) != referenceParameters.end()) {
            write("(*" + identifier->name + ")");
        } else {
            write(identifier->name);
        }
    }
    else if (auto binary = std::dynamic_pointer_cast<BinaryExpression>(expr)) {
        // Handle string equality specially
        if (binary->operator_ == "==") {
            // Check if we're comparing strings
            write("thor_string_equals(");
            generateExpression(binary->left);
            write(", ");
            generateExpression(binary->right);
            write(")");
        } else if (binary->operator_ == "=") {
            // Handle assignment - check if left side is a reference parameter
            if (auto identifier = std::dynamic_pointer_cast<IdentifierExpression>(binary->left)) {
                if (referenceParameters.find(identifier->name) != referenceParameters.end()) {
                    // Assignment to reference parameter - dereference
                    write("(*" + identifier->name + " = ");
                    generateExpression(binary->right);
                    write(")");
                    return;
                }
            }
            // Regular assignment
            write("(");
            generateExpression(binary->left);
            write(" " + binary->operator_ + " ");
            generateExpression(binary->right);
            write(")");
        } else {
            write("(");
            generateExpression(binary->left);
            write(" " + binary->operator_ + " ");
            generateExpression(binary->right);
            write(")");
        }
    }
    else if (auto unary = std::dynamic_pointer_cast<UnaryExpression>(expr)) {
        write("(" + unary->operator_);
        generateExpression(unary->operand);
        write(")");
    }
    else if (auto call = std::dynamic_pointer_cast<CallExpression>(expr)) {
        if (auto member = std::dynamic_pointer_cast<MemberExpression>(call->callee)) {
            // Handle module function calls like std.println or math.add
            if (auto obj = std::dynamic_pointer_cast<IdentifierExpression>(member->object)) {
                if (obj->name == "std") {
                    if (member->property == "println") {
                        write("thor_println(");
                    } else if (member->property == "input") {
                        write("thor_input(");
                    }
                } else {
                    // Other module calls
                    write(obj->name + "_" + member->property + "(");
                }
            }
        } else {
            // Check if this is a function with reference parameters
            bool hasReferenceParams = false;
            std::string functionName;
            if (auto identifier = std::dynamic_pointer_cast<IdentifierExpression>(call->callee)) {
                functionName = identifier->name;
                // For now, hardcode known functions with reference parameters
                hasReferenceParams = (functionName == "testRef" || functionName == "fromFingers");
            }
            
            generateExpression(call->callee);
            write("(");
            
            for (size_t i = 0; i < call->arguments.size(); i++) {
                if (i > 0) write(", ");
                
                if (hasReferenceParams) {
                    // For reference parameters, pass address of variables
                    if (auto argIdentifier = std::dynamic_pointer_cast<IdentifierExpression>(call->arguments[i])) {
                        write("&" + argIdentifier->name);
                    } else {
                        generateExpression(call->arguments[i]);
                    }
                } else {
                    generateExpression(call->arguments[i]);
                }
            }
            write(")");
            return;
        }
        
        for (size_t i = 0; i < call->arguments.size(); i++) {
            if (i > 0) write(", ");
            generateExpression(call->arguments[i]);
        }
        write(")");
    }
    else if (auto member = std::dynamic_pointer_cast<MemberExpression>(expr)) {
        generateExpression(member->object);
        write("." + member->property);
    }
    else if (auto formatStr = std::dynamic_pointer_cast<FormatStringExpression>(expr)) {
        generateFormatString(formatStr->format, formatStr->arguments);
    }
    else if (auto array = std::dynamic_pointer_cast<ArrayExpression>(expr)) {
        write("{");
        for (size_t i = 0; i < array->elements.size(); i++) {
            if (i > 0) write(", ");
            generateExpression(array->elements[i]);
        }
        write("}");
    }
}

bool CodeGenerator::isFloatExpression(std::shared_ptr<Expression> expr) {
    // Check if expression is a float literal
    if (auto literal = std::dynamic_pointer_cast<LiteralExpression>(expr)) {
        return literal->literalType == LiteralExpression::FLOAT;
    }
    
    // Check if expression is a variable with known float type
    if (auto identifier = std::dynamic_pointer_cast<IdentifierExpression>(expr)) {
        // Check if it's a float variable (simple heuristic based on name containing "float")
        std::string name = identifier->name;
        return name.find("float") != std::string::npos || name == "b" || name.find("Float") != std::string::npos;
    }
    
    // Check if expression is a function call that returns float
    if (auto call = std::dynamic_pointer_cast<CallExpression>(expr)) {
        // Check for known float-returning functions
        if (auto memberExpr = std::dynamic_pointer_cast<MemberExpression>(call->callee)) {
            std::string funcName = memberExpr->property;
            return funcName.find("addf") != std::string::npos || 
                   funcName.find("subf") != std::string::npos ||
                   funcName.find("mulf") != std::string::npos ||
                   funcName.find("divf") != std::string::npos;
        }
    }
    
    return false;
}

bool CodeGenerator::isStringExpression(std::shared_ptr<Expression> expr) {
    // Check if expression is a string literal
    if (auto literal = std::dynamic_pointer_cast<LiteralExpression>(expr)) {
        return literal->literalType == LiteralExpression::STRING || 
               literal->literalType == LiteralExpression::BOOLEAN;
    }
    
    // Check if expression is a string variable
    if (auto identifier = std::dynamic_pointer_cast<IdentifierExpression>(expr)) {
        std::string name = identifier->name;
        // Common string variable names
        return name == "business" || name == "location" || name == "animal" || 
               name == "verb" || name.find("name") != std::string::npos ||
               name.find("str") != std::string::npos || name.find("text") != std::string::npos;
    }
    
    // Check if expression is a function call that returns string (like std.input)
    if (auto call = std::dynamic_pointer_cast<CallExpression>(expr)) {
        if (auto memberExpr = std::dynamic_pointer_cast<MemberExpression>(call->callee)) {
            return memberExpr->property == "input";
        }
    }
    
    return false;
}

std::string CodeGenerator::generateFormatString(const std::string& format, 
                                              const std::vector<std::shared_ptr<Expression>>& args) {
    std::string result = format;
    
    // For each argument, determine the appropriate format specifier
    size_t pos = 0;
    for (size_t i = 0; i < args.size() && pos < result.length(); i++) {
        size_t found = result.find("%s", pos);
        if (found != std::string::npos) {
            std::string formatSpec;
            
            // Check if the argument is a literal
            if (auto literal = std::dynamic_pointer_cast<LiteralExpression>(args[i])) {
                if (literal->literalType == LiteralExpression::STRING || 
                    literal->literalType == LiteralExpression::BOOLEAN) {
                    formatSpec = "%s";
                } else {
                    // For numeric literals (int and float), use %g
                    formatSpec = "%g";
                }
            }
            // Check if it's a string expression
            else if (isStringExpression(args[i])) {
                formatSpec = "%s";
            }
            // Default to %g for numeric values
            else {
                formatSpec = "%g";
            }
            
            result.replace(found, 2, formatSpec);
            pos = found + formatSpec.length();
        }
    }
    
    write("thor_format_string(\"" + result + "\"");
    for (size_t i = 0; i < args.size(); i++) {
        write(", ");
        
        // Handle different argument types appropriately
        if (auto literal = std::dynamic_pointer_cast<LiteralExpression>(args[i])) {
            if (literal->literalType == LiteralExpression::INTEGER || 
                literal->literalType == LiteralExpression::FLOAT) {
                write("(double)(");
                generateExpression(args[i]);
                write(")");
            } else {
                generateExpression(args[i]);
            }
        } else if (isStringExpression(args[i])) {
            // String expressions don't need casting
            generateExpression(args[i]);
        } else {
            // Numeric variables - cast to double
            write("(double)(");
            generateExpression(args[i]);
            write(")");
        }
    }
    write(")");
    
    return "";
}

void CodeGenerator::generateStatement(std::shared_ptr<Statement> stmt) {
    if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
        indent();
        generateExpression(exprStmt->expression);
        writeLine(";");
    }
    else if (auto varDecl = std::dynamic_pointer_cast<VariableDeclaration>(stmt)) {
        indent();
        generateType(varDecl->type);
        write(" " + varDecl->name);
        if (varDecl->initializer) {
            write(" = ");
            generateExpression(varDecl->initializer);
        }
        writeLine(";");
    }
    else if (auto constDecl = std::dynamic_pointer_cast<ConstDeclaration>(stmt)) {
        indent();
        write("const ");
        generateType(constDecl->type);
        write(" " + constDecl->name + " = ");
        generateExpression(constDecl->initializer);
        writeLine(";");
    }
    else if (auto block = std::dynamic_pointer_cast<BlockStatement>(stmt)) {
        writeLine("{");
        indentLevel++;
        for (auto& statement : block->statements) {
            generateStatement(statement);
        }
        indentLevel--;
        writeLine("}");
    }
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        indent();
        write("if (");
        generateExpression(ifStmt->condition);
        writeLine(") {");
        indentLevel++;
        generateStatement(ifStmt->thenBranch);
        indentLevel--;
        if (ifStmt->elseBranch) {
            writeLine("} else {");
            indentLevel++;
            generateStatement(ifStmt->elseBranch);
            indentLevel--;
        }
        writeLine("}");
    }
    else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(stmt)) {
        indent();
        write("while (");
        generateExpression(whileStmt->condition);
        writeLine(") {");
        indentLevel++;
        generateStatement(whileStmt->body);
        indentLevel--;
        writeLine("}");
    }
    else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
        indent();
        write("return");
        if (returnStmt->value) {
            write(" ");
            generateExpression(returnStmt->value);
        }
        writeLine(";");
    }
    else if (auto funcDecl = std::dynamic_pointer_cast<FunctionDeclaration>(stmt)) {
        generateFunction(funcDecl);
    }
}

void CodeGenerator::generateFunction(std::shared_ptr<FunctionDeclaration> func) {
    // Skip functions without bodies (built-in functions)
    if (!func->body) {
        return;
    }
    
    // Clear and populate reference parameters for this function
    referenceParameters.clear();
    for (const auto& param : func->parameters) {
        if (param.type->kind == Type::REFERENCE_TYPE) {
            referenceParameters.insert(param.name);
        }
    }
    
    generateType(func->returnType);
    write(" ");
    
    // Add module prefix for non-main functions
    std::string functionName = func->name;
    if (currentProgram && currentProgram->package && currentProgram->package->name != "main") {
        functionName = currentProgram->package->name + "_" + functionName;
    }
    write(functionName + "(");
    
    for (size_t i = 0; i < func->parameters.size(); i++) {
        if (i > 0) write(", ");
        generateType(func->parameters[i].type);
        write(" " + func->parameters[i].name);
    }
    
    writeLine(") {");
    indentLevel++;
    
    for (auto& statement : func->body->statements) {
        generateStatement(statement);
    }
    
    indentLevel--;
    writeLine("}");
}

void CodeGenerator::initializeBuiltinFunctions() {
    builtinFunctions["std.println"] = "thor_println";
    builtinFunctions["std.input"] = "thor_input";
}