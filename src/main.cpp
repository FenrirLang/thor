#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include "Lexer.h"
#include "Parser.h"
#include "ImportProcessor.h"
#include "CodeGenerator.h"

std::string findCCompiler() {
    std::vector<std::string> compilers = {
        "gcc", "clang", "cl", "tcc", "mingw32-gcc"
    };
    
    for (const std::string& compiler : compilers) {
        // Try to run the compiler with --version to see if it exists
        std::string command = compiler + " --version >nul 2>&1";
        if (system(command.c_str()) == 0) {
            return compiler;
        }
    }
    
    // Try to find MinGW in common locations
    std::vector<std::string> mingwPaths = {
        "C:\\MinGW\\bin\\gcc.exe",
        "C:\\MinGW64\\bin\\gcc.exe",
        "C:\\msys64\\mingw64\\bin\\gcc.exe",
        "C:\\msys64\\ucrt64\\bin\\gcc.exe",
        "C:\\Program Files\\mingw-w64\\mingw64\\bin\\gcc.exe"
    };
    
    for (const std::string& path : mingwPaths) {
        if (std::filesystem::exists(path)) {
            return "\"" + path + "\""; // Quote path in case of spaces
        }
    }
    
    return "";
}

bool compileWithCCompiler(const std::string& compiler, const std::string& sourceFile, const std::string& outputFile) {
    std::string command = compiler + " \"" + sourceFile + "\" -o \"" + outputFile + "\"";
    std::cout << "Running: " << command << std::endl;
    
    int result = system(command.c_str());
    return result == 0;
}

void printUsage() {
    std::cout << "Usage: thor <input_file.thor> [output_file.c] [options]\n";
    std::cout << "  input_file.thor  - Thor source file to compile\n";
    std::cout << "  output_file.c    - Output C file (optional, defaults to input name with .c extension)\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --no-compile     - Only generate C code, don't compile to executable\n";
    std::cout << "  --keep-c         - Keep the generated C file after compilation\n";
    std::cout << "  --help           - Show this help message\n";
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    // Check for help flag
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--help") {
            printUsage();
            return 0;
        }
    }
    
    std::string inputFile = argv[1];
    std::string outputFile;
    bool compileExecutable = true;
    bool keepCFile = false;
    
    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-compile") {
            compileExecutable = false;
        } else if (arg == "--keep-c") {
            keepCFile = true;
        } else if (outputFile.empty() && arg.find("--") != 0) {
            // This is the output file argument
            outputFile = arg;
        }
    }
    
    if (outputFile.empty()) {
        // Generate output filename
        std::filesystem::path path(inputFile);
        path.replace_extension(".c");
        outputFile = path.string();
    }
    
    try {
        // Read input file
        std::ifstream file(inputFile);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open input file: " << inputFile << std::endl;
            return 1;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        std::cout << "Compiling " << inputFile << " to " << outputFile << "..." << std::endl;
        
        // Lexical analysis
        Lexer lexer(content);
        auto tokens = lexer.tokenize();
        
        // Syntax analysis
        Parser parser(tokens);
        auto program = parser.parse();
        
        // Process imports
        ImportProcessor importProcessor;
        // Add directory of input file to search paths
        std::filesystem::path inputPath(inputFile);
        if (inputPath.has_parent_path()) {
            importProcessor.addSearchPath(inputPath.parent_path().string());
        }
        
        program = importProcessor.processImports(program);
        auto loadedModules = importProcessor.getLoadedModules();
        
        // Code generation
        CodeGenerator generator;
        std::string generatedCode = generator.generate(program, loadedModules);
        
        // Write output file
        std::ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not create output file: " << outputFile << std::endl;
            return 1;
        }
        
        outFile << generatedCode;
        outFile.close();
        
        std::cout << "Successfully compiled to " << outputFile << std::endl;
        
        // Automatically compile to executable if requested
        if (compileExecutable) {
            std::string compiler = findCCompiler();
            if (compiler.empty()) {
                std::cout << "Warning: No C compiler found. Please install gcc, clang, or MinGW." << std::endl;
                std::cout << "To manually compile: gcc " << outputFile << " -o " 
                         << std::filesystem::path(outputFile).stem().string() << std::endl;
            } else {
                std::cout << "Found C compiler: " << compiler << std::endl;
                
                // Generate executable name
                std::filesystem::path execPath(outputFile);
                execPath.replace_extension(".exe");
                std::string execFile = execPath.string();
                
                if (compileWithCCompiler(compiler, outputFile, execFile)) {
                    std::cout << "Successfully compiled to executable: " << execFile << std::endl;
                    
                    // Delete the C file unless user wants to keep it
                    if (!keepCFile) {
                        try {
                            std::filesystem::remove(outputFile);
                            std::cout << "Cleaned up intermediate file: " << outputFile << std::endl;
                        } catch (const std::exception& e) {
                            std::cout << "Warning: Could not delete intermediate file " << outputFile 
                                     << ": " << e.what() << std::endl;
                        }
                    }
                    
                    std::cout << "To run: " << execFile << std::endl;
                } else {
                    std::cout << "Error: Failed to compile with " << compiler << std::endl;
                    std::cout << "To manually compile: " << compiler << " " << outputFile 
                             << " -o " << std::filesystem::path(outputFile).stem().string() << std::endl;
                    std::cout << "C file preserved for manual compilation." << std::endl;
                }
            }
        } else {
            std::cout << "To manually compile: gcc " << outputFile << " -o " 
                     << std::filesystem::path(outputFile).stem().string() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}