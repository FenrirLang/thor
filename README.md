# Thor Programming Language

Thor is a compiled programming language that transpiles to C code. It features a clean, C-like syntax with built-in functions and modern language constructs.

## Features

- **C-like syntax** with modern improvements
- **Built-in functions** like `std::println()` and `std::print()`
- **Strong typing** with support for `int`, `float`, `string`, `bool`, and `void`
- **Control flow** including `if/else`, `while` loops
- **Function declarations** with parameters and return types
- **Namespace syntax** support (e.g., `std::println`)
- **Import system** for modular programming with duplicate detection
- **Library linking** support for external libraries (GLFW, OpenGL, etc.)
- **Duplicate import warnings** to help keep code clean
- **Transpiles to C** for maximum compatibility and performance

## Language Syntax

### Basic Types
```thor
int x = 42;
float pi = 3.14;
string message = "Hello, World!";
bool flag = true;
```

### Functions
```thor
int add(int a, int b) {
    return a + b;
}

void greet(string name) {
    std::println("Hello, " + name + "!");
}
```

### Control Flow
```thor
if (x > 0) {
    std::println("Positive");
} else {
    std::println("Not positive");
}

while (i < 10) {
    i = i + 1;
}
```

### Imports
```thor
import "mathlib";

int main() {
    int result = add(5, 3);  // Function from mathlib
    return 0;
}
```

### External Function Declarations
```thor
// Declare external C library functions
extern int glfwInit();
extern void glfwTerminate();
extern void* glfwCreateWindow(int width, int height, string title, void* monitor, void* share);
extern void glfwMakeContextCurrent(void* window);
extern int glfwWindowShouldClose(void* window);
extern void glfwSwapBuffers(void* window);
extern void glfwPollEvents();

int main() {
    glfwInit();
    void* window = glfwCreateWindow(800, 600, "My Window", 0, 0);
    glfwMakeContextCurrent(window);
    
    while (glfwWindowShouldClose(window) == 0) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
```

### Built-in Functions
- `std::println(string)` - Print with newline
- `std::print(string)` - Print without newline

## Building the Compiler

### Prerequisites
- CMake 3.20 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)

### Build Steps
```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Compile a Thor source file to C and automatically compile to executable:
```bash
./thor input.thor [output.c] [--no-compile]
```

### Options
- `input.thor` - Thor source file to compile (required)
- `output.c` - Output C file (optional, defaults to input name with `.c` extension)
- `--no-compile` - Skip automatic C compilation step

The compiler will automatically:
1. **Transpile** Thor code to C
2. **Detect** available C compilers (gcc, clang, cl, icc, tcc)
3. **Compile** the C code to an executable
4. **Clean up** intermediate C files (keeps only the executable)
5. **Run** the executable to show output

When using `--no-compile`, the generated C file is preserved and displayed for inspection.

### Supported C Compilers
- **GCC** (GNU Compiler Collection)
- **Clang** (LLVM Compiler)
- **MSVC** (Microsoft Visual C++)
- **ICC** (Intel C++ Compiler)
- **TCC** (Tiny C Compiler)

### Example
```bash
# Compile, run, and clean up automatically (only keeps .exe)
./thor examples/main.thor

# Generate C code only (preserves .c file and shows code)
./thor examples/main.thor --no-compile

# Specify custom output filename (preserves .c when using --no-compile)
./thor examples/main.thor output.c --no-compile
```

## Example Programs

### Hello World (`examples/main.thor`)
```thor
int main() {
    std::println("Hello, World!");
    return 0;
}
```

### Nested Import Example (`examples/nested_import_test.thor`)
```thor
import "mathlib";
import "stringlib";

void demonstrateLibraries() {
    int result = power(3, 3);  // From mathlib
    printNumber(result);       // From stringlib
    std::println("Libraries demonstrated!");
}

int main() {
    std::println("Testing nested imports!");
    demonstrateLibraries();
    return 0;
}
```

### Math Library (`examples/mathlib.thor`)
```thor
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int power(int base, int exp) {
    if (exp == 0) {
        return 1;
    }
    int result = 1;
    int i = 0;
    while (i < exp) {
        result = multiply(result, base);
        i = i + 1;
    }
    return result;
}
```

## Library Linking

Thor supports linking with external libraries through familiar command-line flags:

- **`-l<library>`** - Link against a library (e.g., `-lglfw`, `-lGL`, `-lm`)
- **`-L<path>`** - Add library search path (e.g., `-L/usr/local/lib`)
- **`-I<path>`** - Add include search path (e.g., `-I/usr/local/include`)
- **`--dry-run`** - Show compilation command without executing

### Examples

```bash
# Basic compilation
thor main.thor

# Link with math library
thor calc.thor -lm

# GLFW graphics application
thor game.thor -lglfw -lGL -lm

# Custom output name with libraries
thor app.thor -o myapp -lglfw -L/usr/local/lib -I/usr/local/include

# Preview compilation command
thor graphics.thor -lglfw --dry-run
```

### GLFW Example Output
When compiling with libraries, Thor generates the appropriate C compiler commands:

```
Compiling C code: gcc game.c -o game.exe -I"/usr/local/include" -L"/usr/local/lib" -lglfw -lGL -lm
```

## Compiler Output

When you run the Thor compiler, you'll see:

```
Reading Thor source file: examples/main.thor
Tokenizing...
Found 16 tokens.
Parsing...
Parsing completed successfully.
Generating C code...
Writing C code to: examples/main.c
C code generation completed successfully!

==================================================
Auto-compiling generated C code...
==================================================
Found C compiler: gcc
Compiling C code: gcc examples/main.c -o examples/main.exe
Successfully compiled to: examples/main.exe
Cleaned up intermediate C file: examples/main.c

------------------------------
Running executable...
------------------------------
Hello, World!
------------------------------
```

### Import System with Duplicate Detection

The Thor compiler intelligently handles imports with comprehensive duplicate detection:

```thor
import "mathlib";
import "mathlib";  // WARNING: Duplicate import detected!
```

When duplicate imports are found, Thor displays helpful warnings:

```
WARNING: Duplicate import detected for module 'mathlib'.
         Module already imported from: R:\ThorLang\thor\examples\mathlib.thor
         Found in file: .\examples\duplicate_test.thor
         This import statement will be ignored.
```

The compiler automatically cleans up intermediate C files after successful compilation, keeping only the final executable. When using `--no-compile`, the C file is preserved and the generated code is displayed for inspection.

## Compiler Architecture

The Thor compiler follows a traditional multi-pass design:

1. **Lexical Analysis** (`Lexer.cpp`) - Tokenizes source code
2. **Parsing** (`Parser.cpp`) - Builds Abstract Syntax Tree (AST)
3. **Code Generation** (`CodeGenerator.cpp`) - Converts AST to C code

### Key Components

- **Token.h/cpp** - Token definitions and keyword recognition
- **AST.h/cpp** - Abstract Syntax Tree node definitions
- **Lexer.h/cpp** - Lexical analyzer implementation
- **Parser.h/cpp** - Recursive descent parser
- **CodeGenerator.h/cpp** - C code generation engine
- **main.cpp** - Compiler driver and CLI interface

## Generated C Code

Thor generates clean, readable C code with:
- Standard C includes (`stdio.h`, `stdlib.h`, `string.h`)
- Built-in function implementations
- Proper type mappings (`int` → `int`, `string` → `char*`, `bool` → `int`)
- Preserved function and variable names

## Future Enhancements

- Symbol table and semantic analysis
- Advanced error reporting with line numbers
- Type checking and inference
- Arrays and structures
- More built-in functions and standard library
- Optimization passes
- Debugging information generation

## License

This project is provided as-is for educational and demonstration purposes.
