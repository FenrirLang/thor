#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include "Lexer.h"
#include "Parser.h"
#include "CodeGenerator.h"
#include "ImportProcessor.h"

struct CompilerOptions {
    std::string inputFile;
    std::string outputFile;
    bool autoCompile = true;
    bool dryRun = false;
    std::vector<std::string> libraries;      // -l flags
    std::vector<std::string> libraryPaths;   // -L flags  
    std::vector<std::string> includePaths;   // -I flags
};

std::string readFile(const std::string& filename) 
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeFile(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not create file: " + filename);
    }
    file << content;
}

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " <input.thor> [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -o <file>       - Output executable name (optional)\n";
    std::cout << "  --no-compile    - Skip automatic C compilation step\n";
    std::cout << "  --dry-run       - Show compilation command without executing\n";
    std::cout << "  -l<library>     - Link against library (e.g., -lglfw)\n";
    std::cout << "  -L<path>        - Add library search path (e.g., -L/usr/local/lib)\n";
    std::cout << "  -I<path>        - Add include search path (e.g., -I/usr/local/include)\n";
    std::cout << "  --help          - Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " main.thor\n";
    std::cout << "  " << programName << " main.thor -lglfw -lGL -lm\n";
    std::cout << "  " << programName << " main.thor -o myapp -lglfw -L/usr/local/lib\n";
    std::cout << "  " << programName << " main.thor --dry-run -lglfw -lGL\n";
}

std::string getOutputFilename(const std::string& inputFilename)
{
    size_t lastDot = inputFilename.find_last_of('.');
    if (lastDot != std::string::npos) {
        return inputFilename.substr(0, lastDot) + ".c";
    }
    return inputFilename + ".c";
}

std::string getExecutableFilename(const std::string& inputFilename)
{
    size_t lastDot = inputFilename.find_last_of('.');
    std::string baseName;
    if (lastDot != std::string::npos) {
        baseName = inputFilename.substr(0, lastDot);
    } else {
        baseName = inputFilename;
    }
    
#ifdef _WIN32
    return baseName + ".exe";
#else
    return baseName;
#endif
}

bool isCommandAvailable(const std::string& command)
{
    std::string checkCmd;
#ifdef _WIN32
    checkCmd = "where " + command + " >nul 2>&1";
#else
    checkCmd = "which " + command + " >/dev/null 2>&1";
#endif
    
    return system(checkCmd.c_str()) == 0;
}

std::string findCCompiler()
{
    std::vector<std::string> compilers = {
        "gcc", "clang", "cl", "icc", "tcc"
    };
    
    for (const auto& compiler : compilers) {
        if (isCommandAvailable(compiler)) {
            std::cout << "Found C compiler: " << compiler << std::endl;
            return compiler;
        }
    }
    
    return "";
}

bool compileCode(const std::string& cFile, const std::string& outputFile, const std::string& compiler, const CompilerOptions& options)
{
    std::string compileCmd;
    
    if (compiler == "cl") {
        // Microsoft Visual C++ compiler
        compileCmd = "cl /Fe:" + outputFile + " " + cFile + " /nologo";
        
        // Add include paths
        for (const auto& includePath : options.includePaths) {
            compileCmd += " /I\"" + includePath + "\"";
        }
        
        // Add library paths
        for (const auto& libPath : options.libraryPaths) {
            compileCmd += " /LIBPATH:\"" + libPath + "\"";
        }
        
        // Add libraries (remove -l prefix and add .lib suffix for MSVC)
        for (const auto& lib : options.libraries) {
            std::string libName = lib;
            if (libName.substr(0, 2) == "-l") {
                libName = libName.substr(2) + ".lib";
            }
            compileCmd += " " + libName;
        }
    } else {
        // GCC, Clang, or other Unix-style compilers
        compileCmd = compiler + " " + cFile + " -o " + outputFile;
        
        // Add include paths
        for (const auto& includePath : options.includePaths) {
            compileCmd += " -I\"" + includePath + "\"";
        }
        
        // Add library paths
        for (const auto& libPath : options.libraryPaths) {
            compileCmd += " -L\"" + libPath + "\"";
        }
        
        // Add libraries
        for (const auto& lib : options.libraries) {
            compileCmd += " " + lib;
        }
    }
    
    std::cout << "Compiling C code: " << compileCmd << std::endl;
    
    if (options.dryRun) {
        std::cout << "DRY RUN: Would execute above command" << std::endl;
        return true;  // Pretend success for dry run
    }
    
    int result = system(compileCmd.c_str());
    return result == 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Check for help first
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
    }
    
    CompilerOptions options;
    options.inputFile = argv[1];
    
    // Parse command line arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--no-compile") {
            options.autoCompile = false;
        } else if (arg == "--dry-run") {
            options.dryRun = true;
        } else if (arg == "-o" && i + 1 < argc) {
            options.outputFile = argv[++i];
        } else if (arg.substr(0, 2) == "-l") {
            options.libraries.push_back(arg);
        } else if (arg.substr(0, 2) == "-L") {
            options.libraryPaths.push_back(arg.substr(2));
        } else if (arg.substr(0, 2) == "-I") {
            options.includePaths.push_back(arg.substr(2));
        } else if (options.outputFile.empty() && arg.find("--") != 0 && arg.find("-") != 0) {
            // Legacy support for positional output file argument
            options.outputFile = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Set default output file if not specified
    if (options.outputFile.empty()) {
        options.outputFile = getExecutableFilename(options.inputFile);
    }
    
    std::string cFile = getOutputFilename(options.inputFile);
    
    try {
        // Read Thor source code
        std::cout << "Reading Thor source file: " << options.inputFile << std::endl;
        std::string sourceCode = readFile(options.inputFile);
        
        // Lexical analysis
        std::cout << "Tokenizing..." << std::endl;
        Thor::Lexer lexer(sourceCode);
        std::vector<Thor::Token> tokens = lexer.tokenize();
        
        std::cout << "Found " << tokens.size() << " tokens." << std::endl;
        
        // Parsing
        std::cout << "Parsing..." << std::endl;
        Thor::Parser parser(tokens);
        auto program = parser.parse();
        
        if (!program) {
            std::cerr << "Failed to parse the program." << std::endl;
            return 1;
        }
        
        std::cout << "Parsing completed successfully." << std::endl;
        
        // Process imports
        std::cout << "Processing imports..." << std::endl;
        Thor::ImportProcessor importProcessor;
        auto processedProgram = importProcessor.processImports(std::move(program), options.inputFile);
        
        if (!processedProgram) {
            std::cerr << "Failed to process imports." << std::endl;
            return 1;
        }
        
        std::cout << "Import processing completed successfully." << std::endl;
        
        // Code generation
        std::cout << "Generating C code..." << std::endl;
        Thor::CodeGenerator generator;
        std::string cCode = generator.generate(processedProgram.get());
        
        // Write output
        std::cout << "Writing C code to: " << cFile << std::endl;
        writeFile(cFile, cCode);
        
        std::cout << "C code generation completed successfully!" << std::endl;
        
        // Automatic compilation
        if (options.autoCompile) {
            std::cout << "\n" << std::string(50, '=') << std::endl;
            std::cout << "Auto-compiling generated C code..." << std::endl;
            std::cout << std::string(50, '=') << std::endl;
            
            std::string compiler = findCCompiler();
            if (compiler.empty()) {
                std::cout << "Warning: No C compiler found on system." << std::endl;
                std::cout << "Available compilers checked: gcc, clang, cl, icc, tcc" << std::endl;
                std::cout << "Please install a C compiler to enable automatic compilation." << std::endl;
            } else {
                
                if (compileCode(cFile, options.outputFile, compiler, options)) {
                    std::cout << "Successfully compiled to: " << options.outputFile << std::endl;
                    
                    if (!options.dryRun) {
                        // Clean up the C file after successful compilation
                        try {
                            std::filesystem::remove(cFile);
                            std::cout << "Cleaned up intermediate C file: " << cFile << std::endl;
                        } catch (const std::exception& e) {
                            std::cout << "Warning: Could not remove C file " << cFile << ": " << e.what() << std::endl;
                        }
                        
                        // Try to run the executable if it's a main program
                        if (std::filesystem::exists(options.outputFile)) {
                            std::cout << "\n" << std::string(30, '-') << std::endl;
                            std::cout << "Running executable..." << std::endl;
                            std::cout << std::string(30, '-') << std::endl;
                            
                            std::string runCmd;
#ifdef _WIN32
                            runCmd = options.outputFile;
#else
                            runCmd = "./" + options.outputFile;
#endif
                            system(runCmd.c_str());
                            std::cout << std::string(30, '-') << std::endl;
                        }
                    }
                } else {
                    std::cerr << "Failed to compile C code." << std::endl;
                    return 1;
                }
            }
        }
        
        // Only show generated C code when not auto-compiling or when requested
        if (!options.autoCompile) {
            std::cout << "\nGenerated C code:\n" << std::string(50, '-') << std::endl;
            std::cout << cCode << std::endl;
            std::cout << std::string(50, '-') << std::endl;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}