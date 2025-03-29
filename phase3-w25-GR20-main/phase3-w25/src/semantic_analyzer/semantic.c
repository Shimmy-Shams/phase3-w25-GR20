#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "parser.h"

// ============================
// Symbol Table Implementation
// ============================

// Create a new symbol table with scope level 0
SymbolTable* init_symbol_table() {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (table) {
        table->head = NULL;
        table->current_scope = 0;
    }
    return table;
}

// Add a new symbol to the table (in the current scope)
void add_symbol(SymbolTable* table, const char* name, int type, int line) {
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    if (symbol) {
        strncpy(symbol->name, name, sizeof(symbol->name));
        symbol->name[sizeof(symbol->name) - 1] = '\0';
        symbol->type = type;
        symbol->scope_level = table->current_scope;
        symbol->line_declared = line;
        symbol->is_initialized = 0;
        // Insert at the beginning of the list
        symbol->next = table->head;
        table->head = symbol;
    }
}

// Look up a symbol by name across all scopes
Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

// Look up a symbol by name only in the current scope
Symbol* lookup_symbol_current_scope(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0 && current->scope_level == table->current_scope)
            return current;
        current = current->next;
    }
    return NULL;
}

// Enter a new scope level (e.g., when entering a block)
void enter_scope(SymbolTable* table) {
    table->current_scope++;
}

// Remove all symbols declared in the current scope
void remove_symbols_in_current_scope(SymbolTable* table) {
    Symbol** current = &table->head;
    while (*current) {
        if ((*current)->scope_level == table->current_scope) {
            Symbol* temp = *current;
            *current = (*current)->next;
            free(temp);
        } else {
            current = &((*current)->next);
        }
    }
}

// Exit the current scope and clean up symbols no longer in scope
void exit_scope(SymbolTable* table) {
    remove_symbols_in_current_scope(table);
    table->current_scope--;
}

// Free the entire symbol table
void free_symbol_table(SymbolTable* table) {
    Symbol* current = table->head;
    while (current) {
        Symbol* temp = current;
        current = current->next;
        free(temp);
    }
    free(table);
}

// Dump the contents of the symbol table (for debugging)
void dump_symbol_table(SymbolTable* table) {
    int count = 0;
    Symbol* current = table->head;
    while (current) {
        count++;
        current = current->next;
    }
    printf("== SYMBOL TABLE DUMP ==\n");
    printf("Total symbols: %d\n\n", count);
    current = table->head;
    int index = 0;
    while (current) {
        printf("Symbol[%d]:\n", index);
        printf("  Name: %s\n", current->name);
        printf("  Type: %d\n", current->type);
        printf("  Scope Level: %d\n", current->scope_level);
        printf("  Line Declared: %d\n", current->line_declared);
        printf("  Initialized: %s\n", current->is_initialized ? "Yes" : "No");
        printf("\n");
        current = current->next;
        index++;
    }
    printf("===================\n");
}

// ============================
// Semantic Error Reporting
// ============================
void semantic_error(SemanticErrorType error, const char* name, int line) {
    printf("Semantic Error at line %d: ", line);
    switch (error) {
        case SEM_ERROR_UNDECLARED_VARIABLE:
            printf("Undeclared variable '%s'\n", name);
            break;
        case SEM_ERROR_REDECLARED_VARIABLE:
            printf("Variable '%s' already declared in this scope\n", name);
            break;
        case SEM_ERROR_TYPE_MISMATCH:
            printf("Type mismatch involving '%s'\n", name);
            break;
        case SEM_ERROR_UNINITIALIZED_VARIABLE:
            printf("Variable '%s' may be used uninitialized\n", name);
            break;
        case SEM_ERROR_INVALID_OPERATION:
            printf("Invalid operation involving '%s'\n", name);
            break;
        default:
            printf("Unknown semantic error with '%s'\n", name);
    }
}

// ============================
// Semantic Analysis Functions
// ============================

// Main semantic analysis function; initializes symbol table and checks the AST
int analyze_semantics(ASTNode* ast) {
    SymbolTable* table = init_symbol_table();
    int result = check_program(ast, table);
    dump_symbol_table(table);  // Dump the table (optional for debugging)
    free_symbol_table(table);
    return result;
}

// Check the overall program (assumes AST_PROGRAM as the root)
int check_program(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;
    int valid = 1;
    if (node->type == AST_PROGRAM) {
        if (node->left)
            valid &= check_statement(node->left, table);
        if (node->right)
            valid &= check_program(node->right, table);
    } else {
        valid &= check_statement(node, table);
    }
    return valid;
}

// Dispatch a statement node based on its type
int check_statement(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;
    int valid = 1;
    switch (node->type) {
        case AST_VARDECL:
            valid = check_declaration(node, table);
            break;
        case AST_ASSIGN:
            valid = check_assignment(node, table);
            break;
        case AST_BLOCK:
            valid = check_block(node, table);
            break;
        case AST_IF:
            // For if: left is condition; right is then-branch; else_branch (if not NULL) is the else-part.
            if (node->left)
                valid &= check_condition(node->left, table);
            if (node->right)
                valid &= check_statement(node->right, table);
            if (node->else_branch)
                valid &= check_statement(node->else_branch, table);
            break;
        case AST_WHILE:
            if (node->left)
                valid &= check_condition(node->left, table);
            if (node->right)
                valid &= check_statement(node->right, table);
            break;
        case AST_PRINT:
            if (node->left)
                valid &= check_expression(node->left, table);
            break;
        default:
            // For any other node types, recursively check children if applicable
            valid &= check_expression(node, table);
            break;
    }
    return valid;
}

// Check a variable declaration node
int check_declaration(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_VARDECL)
        return 1;
    const char* name = node->token.lexeme;
    // Ensure the variable is not already declared in the same scope
    Symbol* existing = lookup_symbol_current_scope(table, name);
    if (existing) {
        semantic_error(SEM_ERROR_REDECLARED_VARIABLE, name, node->token.line);
        return 0;
    }
    // Add the new variable; adjust the type (e.g., TOKEN_INT) as needed
    add_symbol(table, name, TOKEN_INT, node->token.line);
    return 1;
}

// Check an assignment node
int check_assignment(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_ASSIGN || !node->left || !node->right)
        return 0;
    const char* name = node->left->token.lexeme;
    Symbol* symbol = lookup_symbol(table, name);
    if (!symbol) {
        semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, name, node->token.line);
        return 0;
    }
    int expr_valid = check_expression(node->right, table);
    if (expr_valid) {
        symbol->is_initialized = 1;
    }
    return expr_valid;
}

// Recursively check an expression node for semantic correctness
int check_expression(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;
    int valid = 1;
    switch (node->type) {
        case AST_NUMBER:
            // Literals are always valid
            break;
        case AST_IDENTIFIER:
            {
                Symbol* symbol = lookup_symbol(table, node->token.lexeme);
                if (!symbol) {
                    semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, node->token.lexeme, node->token.line);
                    valid = 0;
                } else if (!symbol->is_initialized) {
                    semantic_error(SEM_ERROR_UNINITIALIZED_VARIABLE, node->token.lexeme, node->token.line);
                }
            }
            break;
        case AST_BINOP:
            valid &= check_expression(node->left, table);
            valid &= check_expression(node->right, table);
            // Additional type-checking can be inserted here.
            break;
        default:
            valid &= check_expression(node->left, table);
            valid &= check_expression(node->right, table);
            break;
    }
    return valid;
}

// Check a block of statements (handles scope entry/exit)
// Assumes a block node where the left child is the first statement and the right child continues the block.
int check_block(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;
    enter_scope(table);
    int valid = 1;
    valid &= check_statement(node->left, table);
    valid &= check_block(node->right, table);
    exit_scope(table);
    return valid;
}

// Check a condition expression (used in if/while)
int check_condition(ASTNode* node, SymbolTable* table) {
    // Conditions are expressions – extend with boolean type checks if needed.
    return check_expression(node, table);
}



