#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"   // For the ASTNode structure
#include "tokens.h"   // For token definitions (e.g., TOKEN_INT)

/* ============================
   Symbol Table Structures
   ============================ */
typedef struct Symbol {
    char name[100];           // Identifier name
    int type;                 // Data type (e.g., TOKEN_INT)
    int scope_level;          // Nesting scope level
    int line_declared;        // Line number where declared
    int is_initialized;       // Flag: 0 = not initialized, 1 = initialized
    struct Symbol* next;      // Pointer for linked list implementation
} Symbol;

typedef struct {
    Symbol* head;             // Head of the symbol list
    int current_scope;        // Current scope level
} SymbolTable;

/* ============================
   Symbol Table Operations
   ============================ */
SymbolTable* init_symbol_table();
void add_symbol(SymbolTable* table, const char* name, int type, int line);
Symbol* lookup_symbol(SymbolTable* table, const char* name);
Symbol* lookup_symbol_current_scope(SymbolTable* table, const char* name);
void enter_scope(SymbolTable* table);
void exit_scope(SymbolTable* table);
void remove_symbols_in_current_scope(SymbolTable* table);
void free_symbol_table(SymbolTable* table);
void dump_symbol_table(SymbolTable* table);  // For debugging and table dump

/* ============================
   Semantic Error Reporting
   ============================ */
typedef enum {
    SEM_ERROR_NONE,
    SEM_ERROR_UNDECLARED_VARIABLE,
    SEM_ERROR_REDECLARED_VARIABLE,
    SEM_ERROR_TYPE_MISMATCH,
    SEM_ERROR_UNINITIALIZED_VARIABLE,
    SEM_ERROR_INVALID_OPERATION,
    SEM_ERROR_SEMANTIC_ERROR
} SemanticErrorType;

void semantic_error(SemanticErrorType error, const char* name, int line);

/* ============================
   Semantic Analysis Functions
   ============================ */
int analyze_semantics(ASTNode* ast);
int check_program(ASTNode* node, SymbolTable* table);
int check_statement(ASTNode* node, SymbolTable* table);
int check_declaration(ASTNode* node, SymbolTable* table);
int check_assignment(ASTNode* node, SymbolTable* table);
int check_expression(ASTNode* node, SymbolTable* table);
int check_block(ASTNode* node, SymbolTable* table);
int check_condition(ASTNode* node, SymbolTable* table);

#endif // SEMANTIC_H
