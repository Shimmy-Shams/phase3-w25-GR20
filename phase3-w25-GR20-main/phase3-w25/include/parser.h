#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"

// AST Node types for our language constructs.
typedef enum {
    AST_PROGRAM,        // Program node (sequence of statements)
    AST_VARDECL,        // Variable declaration (e.g., int x)
    AST_ASSIGN,         // Assignment (e.g., x = 5)
    AST_PRINT,          // Print statement
    AST_NUMBER,         // Number literal
    AST_IDENTIFIER,     // Variable or function name
    AST_BINOP,          // Binary operator node (e.g., +, -, *, /, <, >, etc.)
    AST_IF,             // If statement
    AST_WHILE,          // While loop
    AST_REPEAT,         // Repeat-until loop
    AST_BLOCK,          // Block of statements: { ... }
    AST_FUNCALL         // Function call (e.g., factorial(x))
} ASTNodeType;

// Extended parse error codes.
typedef enum {
    PARSE_ERROR_NONE,
    PARSE_ERROR_UNEXPECTED_TOKEN,
    PARSE_ERROR_MISSING_SEMICOLON,
    PARSE_ERROR_MISSING_IDENTIFIER,
    PARSE_ERROR_MISSING_EQUALS,
    PARSE_ERROR_INVALID_EXPRESSION,
    PARSE_ERROR_MISSING_LPAREN,
    PARSE_ERROR_MISSING_RPAREN,
    PARSE_ERROR_MISSING_CONDITION,
    PARSE_ERROR_MISSING_BLOCK,
    PARSE_ERROR_INVALID_OPERATOR,
    PARSE_ERROR_FUNCTION_CALL
} ParseError;

// AST Node structure.
typedef struct ASTNode {
    ASTNodeType type;           // Node type
    Token token;                // Associated token (useful for error messages)
    struct ASTNode* left;       // Left child (e.g., condition, first statement)
    struct ASTNode* right;      // Right child (e.g., else branch or next statement)
} ASTNode;

// Parser interface functions.
void parser_init(const char* input);
ASTNode* parse(void);
void print_ast(ASTNode* node, int level);
void free_ast(ASTNode* node);

#endif /* PARSER_H */
