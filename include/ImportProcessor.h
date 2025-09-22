#pragma once
#include "AST.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>

namespace Thor {

class ImportProcessor {
private:
    std::unordered_set<std::string> processedFiles;
    std::vector<std::string> searchPaths;
    
    std::string findModule(const std::string& moduleName, const std::string& currentDir);
    std::string readFile(const std::string& filename);
    std::unique_ptr<Program> parseFile(const std::string& filename);
    
public:
    ImportProcessor();
    void addSearchPath(const std::string& path);
    
    // Process imports and return a merged program
    std::unique_ptr<Program> processImports(std::unique_ptr<Program> mainProgram, const std::string& mainFile);
    
    // Get all processed files (for dependency tracking)
    const std::unordered_set<std::string>& getProcessedFiles() const;
};

} // namespace Thor