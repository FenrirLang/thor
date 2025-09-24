#pragma once
#include "AST.h"
#include <string>
#include <unordered_map>
#include <vector>

class ImportProcessor {
private:
    std::unordered_map<std::string, std::shared_ptr<Program>> moduleCache;
    std::vector<std::string> searchPaths;
    
    std::string resolveModulePath(const std::string& module) const;
    std::shared_ptr<Program> loadModule(const std::string& module);
    
public:
    ImportProcessor();
    void addSearchPath(const std::string& path);
    std::shared_ptr<Program> processImports(std::shared_ptr<Program> program);
    std::unordered_map<std::string, std::shared_ptr<Program>> getLoadedModules() const;
};