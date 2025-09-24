#include "ImportProcessor.h"
#include "Lexer.h"
#include "Parser.h"
#include <filesystem>
#include <fstream>
#include <iostream>

ImportProcessor::ImportProcessor() {
    // Add default search paths
    searchPaths.push_back(".");
    searchPaths.push_back("./example");
}

void ImportProcessor::addSearchPath(const std::string& path) {
    searchPaths.push_back(path);
}

std::shared_ptr<Program> ImportProcessor::processImports(std::shared_ptr<Program> program) {
    // Load all imported modules
    for (auto& import : program->imports) {
        loadModule(import->module);
    }
    
    return program;
}

std::string ImportProcessor::resolveModulePath(const std::string& module) const {
    // Try to find the module file
    for (const auto& searchPath : searchPaths) {
        std::filesystem::path modulePath = std::filesystem::path(searchPath) / (module + ".thor");
        if (std::filesystem::exists(modulePath)) {
            return modulePath.string();
        }
        
        // Also try with subdirectories
        std::filesystem::path moduleSubPath = std::filesystem::path(searchPath) / module / (module + ".thor");
        if (std::filesystem::exists(moduleSubPath)) {
            return moduleSubPath.string();
        }
        
        // Try looking for directory with same name containing files
        std::filesystem::path moduleDir = std::filesystem::path(searchPath) / module;
        if (std::filesystem::exists(moduleDir) && std::filesystem::is_directory(moduleDir)) {
            // Look for any .thor files in the directory
            for (const auto& entry : std::filesystem::directory_iterator(moduleDir)) {
                if (entry.path().extension() == ".thor") {
                    return entry.path().string();
                }
            }
        }
    }
    
    throw std::runtime_error("Could not find module: " + module);
}

std::shared_ptr<Program> ImportProcessor::loadModule(const std::string& module) {
    // Check if already loaded
    if (moduleCache.find(module) != moduleCache.end()) {
        return moduleCache[module];
    }
    
    // Handle built-in modules
    if (module == "std.io") {
        // Create a virtual std.io module
        auto stdProgram = std::make_shared<Program>();
        stdProgram->package = std::make_shared<PackageDeclaration>("std");
        
        // Add println function declaration
        std::vector<Parameter> printlnParams;
        printlnParams.emplace_back("message", Type::createString());
        auto printlnFunc = std::make_shared<FunctionDeclaration>(
            "println", printlnParams, Type::createVoid(), nullptr);
        stdProgram->statements.push_back(printlnFunc);
        
        // Add input function declaration
        std::vector<Parameter> inputParams;
        inputParams.emplace_back("prompt", Type::createString());
        auto inputFunc = std::make_shared<FunctionDeclaration>(
            "input", inputParams, Type::createString(), nullptr);
        stdProgram->statements.push_back(inputFunc);
        
        moduleCache[module] = stdProgram;
        std::cout << "Loaded built-in module: " << module << std::endl;
        return stdProgram;
    }
    
    try {
        std::string filePath = resolveModulePath(module);
        
        // Read file
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open module file: " + filePath);
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        // Parse module
        Lexer lexer(content);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto moduleProgram = parser.parse();
        
        // Cache the module
        moduleCache[module] = moduleProgram;
        
        // Recursively load imports from this module
        processImports(moduleProgram);
        
        std::cout << "Loaded module: " << module << " from " << filePath << std::endl;
        return moduleProgram;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Error loading module '" + module + "': " + e.what());
    }
}

std::unordered_map<std::string, std::shared_ptr<Program>> ImportProcessor::getLoadedModules() const {
    return moduleCache;
}