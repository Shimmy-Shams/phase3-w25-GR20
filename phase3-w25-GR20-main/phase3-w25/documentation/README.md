# Phase 3: Semantic Analyzer

This repository contains the implementation of **Phase 3 (Semantic Analysis)** of our compiler project. In this phase, the previously built lexical analyzer and parser are extended with semantic analysis capabilities. The semantic analyzer ensures that the source code is logically valid, adheres to the language's scope rules, and maintains type consistency.

## Purpose
The semantic analyzer performs the following key tasks:

- **Symbol Table Management:** Tracks variable declarations, scope levels, types, and initialization statuses.
- **Semantic Checks:** Validates declarations, assignments, and expressions to ensure semantic correctness.
- **Error Reporting:** Provides clear, detailed error messages for semantic violations such as undeclared variables, redeclarations, and uninitialized variables.

## Repository Structure

| Directory/File                      | Description                         |
|------------------------------------|-------------------------------------|
| `include/lexer.h`                  | Lexer header                        |
| `include/parser.h`                 | Parser header                       |
| `include/semantic.h`               | Semantic analyzer header            |
| `include/tokens.h`                 | Token definitions                   |
| `src/lexer/lexer.c`                | Lexer implementation                |
| `src/parser/parser.c`              | Parser implementation               |
| `src/semantic_analyzer/semantic.c` | Semantic analyzer implementation    |
| `test/input_valid.txt`             | Valid test cases                    |
| `test/input_invalid.txt`           | Invalid test cases for error checks |


## How to Compile and Run

### 1. Compile the Program

Use `gcc` to compile the project:

```bash
gcc -I include src/lexer/lexer.c src/parser/parser.c src/semantic_analyzer/semantic.c -o phase3
```

### 2. Run the Program
Execute the compiled binary:

```bash
./phase3
```

This will automatically read and analyze the test files located in the `test/` directory.

### Test Files

- **`input_valid.txt`**  
  Contains code examples that should pass semantic analysis without errors.

- **`input_invalid.txt`**  
  Contains code examples designed to trigger semantic errors and demonstrate the error-reporting capabilities.
