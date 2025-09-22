#include "ImportProcessor.h"
#include "Lexer.h"
#include "Parser.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <functional>

namespace Thor {

ImportProcessor::ImportProcessor() {
    // Add default search paths
    searchPaths.push_back(".");  // Current directory
    searchPaths.push_back("./lib");  // Local lib directory
}

void ImportProcessor::addSearchPath(const std::string& path) {
    searchPaths.push_back(path);
}

std::string ImportProcessor::findModule(const std::string& moduleName, const std::string& currentDir) {
    std::vector<std::string> candidates;
    
    // Try relative to current file
    candidates.push_back(currentDir + "/" + moduleName);
    candidates.push_back(currentDir + "/" + moduleName + ".thor");
    
    // Try search paths
    for (const auto& searchPath : searchPaths) {
        candidates.push_back(searchPath + "/" + moduleName);
        candidates.push_back(searchPath + "/" + moduleName + ".thor");
    }
    
    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            // Normalize the path to ensure consistent comparisons
            return std::filesystem::canonical(candidate).string();
        }
    }
    
    throw std::runtime_error("Module not found: " + moduleName);
}

std::string ImportProcessor::readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::unique_ptr<Program> ImportProcessor::parseFile(const std::string& filename) {
    std::string sourceCode = readFile(filename);
    
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();
    
    Parser parser(tokens);
    return parser.parse();
}

std::unique_ptr<Program> ImportProcessor::processImports(std::unique_ptr<Program> mainProgram, const std::string& mainFile) {
    processedFiles.clear();
    
    std::string mainDir = std::filesystem::path(mainFile).parent_path().string();
    auto mergedProgram = std::make_unique<Program>();
    
    // Process imports recursively
    std::function<void(Program*, const std::string&, const std::string&)> processProgram = 
        [&](Program* program, const std::string& currentDir, const std::string& currentFile) {
            for (auto& stmt : program->statements) {
                if (auto importStmt = dynamic_cast<ImportStatement*>(stmt.get())) {
                    try {
                        std::string modulePath = findModule(importStmt->moduleName, currentDir);
                        
                        // Avoid circular imports and duplicate imports
                        if (processedFiles.find(modulePath) == processedFiles.end()) {
                            processedFiles.insert(modulePath);
                            
                            std::cout << "Processing import: " << importStmt->moduleName 
                                      << " -> " << modulePath << std::endl;
                            
                            auto importedProgram = parseFile(modulePath);
                            std::string importDir = std::filesystem::path(modulePath).parent_path().string();
                            
                            // Recursively process imports in the imported file
                            processProgram(importedProgram.get(), importDir, modulePath);
                            
                            // Add non-import statements from imported file
                            for (auto& importedStmt : importedProgram->statements) {
                                if (!dynamic_cast<ImportStatement*>(importedStmt.get())) {
                                    mergedProgram->addStatement(std::move(importedStmt));
                                }
                            }
                        } else {
                            std::cout << "WARNING: Duplicate import detected for module '" 
                                      << importStmt->moduleName << "'." << std::endl;
                            std::cout << "         Module already imported from: " << modulePath << std::endl;
                            std::cout << "         Found in file: " << currentFile << std::endl;
                            std::cout << "         This import statement will be ignored." << std::endl;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Import error: " << e.what() << std::endl;
                        throw;
                    }
                } else {
                    // Non-import statement - will be added later
                }
            }
        };
    
    // Process imports in main program
    processProgram(mainProgram.get(), mainDir, mainFile);
    
    // Add non-import statements from main program
    for (auto& stmt : mainProgram->statements) {
        if (!dynamic_cast<ImportStatement*>(stmt.get())) {
            mergedProgram->addStatement(std::move(stmt));
        }
    }
    
    return mergedProgram;
}

const std::unordered_set<std::string>& ImportProcessor::getProcessedFiles() const {
    return processedFiles;
}

} // namespace Thor