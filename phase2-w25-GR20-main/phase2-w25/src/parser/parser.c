/* parser.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"

// -----------------------------------------------------------------
// Forward Declarations for New Statement Types
// -----------------------------------------------------------------
static ASTNode* parse_if_statement(void);
static ASTNode* parse_while_statement(void);
static ASTNode* parse_repeat_statement(void);
static ASTNode* parse_print_statement(void);
static ASTNode* parse_block(void);
static ASTNode* parse_factorial(void);

// -----------------------------------------------------------------
// Forward Declarations for Expression and Statement Parsing
// -----------------------------------------------------------------
static ASTNode *parse_expression(void);
static ASTNode *parse_statement(void);

// -----------------------------------------------------------------
// Global Variables for Parser State
// -----------------------------------------------------------------
static Token current_token;
static int position = 0;
static const char *source;

// -----------------------------------------------------------------
// Extended Parse Error Reporting
// -----------------------------------------------------------------
static void dbg() {
    printf("%d - %d - %s\n", current_token.line, current_token.type, current_token.lexeme);
}

static void parse_error(ParseError error, Token token) {
    printf("Parse Error at line %d: ", token.line);
    switch (error) {
        case PARSE_ERROR_UNEXPECTED_TOKEN:
            printf("Unexpected token '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_SEMICOLON:
            printf("Missing semicolon after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_IDENTIFIER:
            printf("Expected identifier after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_EQUALS:
            printf("Expected '=' after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_EXPRESSION:
            printf("Invalid expression after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_LPAREN:
            printf("Missing '(' after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_RPAREN:
            printf("Missing ')' after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_CONDITION:
            printf("Missing condition after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_BLOCK:
            printf("Missing block braces after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_OPERATOR:
            printf("Invalid operator '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_FUNCTION_CALL:
            printf("Function call error near '%s'\n", token.lexeme);
            break;
        default:
            printf("Unknown error\n");
    }
}

// -----------------------------------------------------------------
// Basic Parser Utilities
// -----------------------------------------------------------------

// Consume the current token and move to the next one.
static void advance(void) {
    current_token = get_next_token(source, &position);
}

// Create a new AST node.
static ASTNode *create_node(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node) {
        node->type = type;
        node->token = current_token;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

// Check if the current token matches the expected type.
static int match(TokenType type) {
    return current_token.type == type;
}

// Expect a token of a given type or report an error.
static void expect(TokenType type) {
    if (match(type)) {
        advance();
    } else {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        exit(1); // Could implement error recovery instead.
    }
}

// -----------------------------------------------------------------
// Expression Parsing with Operator Precedence
// -----------------------------------------------------------------

// Forward declarations for expression helper functions.
static ASTNode *parse_primary(void);
static ASTNode *parse_factor(void);
static ASTNode *parse_term(void);
static ASTNode *parse_comparison(void);
static ASTNode *parse_equality(void);

// parse_primary: Handles numbers, identifiers (or function calls), and parenthesized expressions.
static ASTNode *parse_primary(void) {
    ASTNode *node;
    if (match(TOKEN_NUMBER)) {
        node = create_node(AST_NUMBER);
        advance();
        return node;
    } else if (match(TOKEN_IDENTIFIER)) {
        // Look ahead to determine if this is a function call.
        int saved_position = position;
        Token next_token = get_next_token(source, &position);
        position = saved_position; // Restore position
        if (next_token.type == TOKEN_LPAREN && strcmp(current_token.lexeme, "factorial") == 0) {
            return parse_factorial();
        } else {
            node = create_node(AST_IDENTIFIER);
            advance();
            return node;
        }
    } else if (match(TOKEN_LPAREN)) {
        advance(); // consume '('
        node = parse_expression();
        expect(TOKEN_RPAREN); // expect ')'
        return node;
    } else {
        printf("Syntax Error: Expected primary expression at line %d\n", current_token.line);
        exit(1);
    }
}

// parse_factor: Handles multiplication and division.
static ASTNode *parse_factor(void) {
    ASTNode *node = parse_primary();
    while (match(TOKEN_OPERATOR) && (current_token.lexeme[0] == '*' || current_token.lexeme[0] == '/')) {
        Token op = current_token;
        advance(); // consume operator
        ASTNode *right = parse_primary();
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = op;
        new_node->left = node;
        new_node->right = right;
        node = new_node;
    }
    return node;
}

// parse_term: Handles addition and subtraction.
static ASTNode *parse_term(void) {
    ASTNode *node = parse_factor();
    while (match(TOKEN_OPERATOR) && (current_token.lexeme[0] == '+' || current_token.lexeme[0] == '-')) {
        Token op = current_token;
        advance(); // consume operator
        ASTNode *right = parse_factor();
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = op;
        new_node->left = node;
        new_node->right = right;
        node = new_node;
    }
    return node;
}

// parse_comparison: Handles relational operators like '<' and '>'.
static ASTNode *parse_comparison(void) {
    ASTNode *node = parse_term();
    while (match(TOKEN_OPERATOR) &&
           (strcmp(current_token.lexeme, "<") == 0 || strcmp(current_token.lexeme, ">") == 0)) {
        Token op = current_token;
        advance(); // consume operator
        ASTNode *right = parse_term();
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = op;
        new_node->left = node;
        new_node->right = right;
        node = new_node;
    }
    return node;
}

// parse_equality: Handles equality operators (== and !=).
static ASTNode *parse_equality(void) {
    ASTNode *node = parse_comparison();
    while (match(TOKEN_OPERATOR) &&
           (strcmp(current_token.lexeme, "==") == 0 || strcmp(current_token.lexeme, "!=") == 0)) {
        Token op = current_token;
        advance(); // consume operator
        ASTNode *right = parse_comparison();
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = op;
        new_node->left = node;
        new_node->right = right;
        node = new_node;
    }
    return node;
}

// The top-level parse_expression now uses our equality parsing.
static ASTNode *parse_expression(void) {
    ASTNode* node = parse_equality();
    return node;
}

// -----------------------------------------------------------------
// Statement Parsing Functions
// -----------------------------------------------------------------

// Parse variable declaration: int x;
static ASTNode *parse_declaration(void) {
    ASTNode *node = create_node(AST_VARDECL);
    advance(); // consume 'int'
    if (!match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, current_token);
        exit(1);
    }
    node->token = current_token;
    advance();
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        exit(1);
    }
    advance();
    return node;
}

// Parse assignment: x = expression;
static ASTNode *parse_assignment(void) {
    ASTNode *node = create_node(AST_ASSIGN);
    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;
    advance();
    if (!match(TOKEN_EQUALS)) {
        parse_error(PARSE_ERROR_MISSING_EQUALS, current_token);
        exit(1);
    }
    advance();
    node->right = parse_expression();
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        exit(1);
    }
    advance();
    return node;
}

// Parse if statement: if (condition) statement
static ASTNode *parse_if_statement(void) {
    ASTNode *node = create_node(AST_IF);
    advance(); // consume 'if'
    expect(TOKEN_LPAREN);  // expect '('
    node->left = parse_expression(); // condition stored in left child
    expect(TOKEN_RPAREN);  // expect ')'
    node->right = parse_statement(); // then-branch
    return node;
}

// Parse while loop: while (condition) statement
static ASTNode *parse_while_statement(void) {
    ASTNode *node = create_node(AST_WHILE);
    advance(); // consume 'while'
    expect(TOKEN_LPAREN);
    node->left = parse_expression(); // condition
    expect(TOKEN_RPAREN);
    node->right = parse_statement(); // loop body
    return node;
}

// Parse repeat-until loop: repeat statement until (condition)
static ASTNode *parse_repeat_statement(void) {
    ASTNode *node = create_node(AST_REPEAT);
    advance(); // consume 'repeat'
    node->left = parse_statement(); // repeat body
    if (!match(TOKEN_UNTIL)) {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        exit(1);
    }
    advance(); // consume 'until'
    expect(TOKEN_LPAREN);
    node->right = parse_expression(); // condition
    expect(TOKEN_RPAREN);
    expect(TOKEN_SEMICOLON);
    return node;
}

// Parse print statement: print expression;
static ASTNode *parse_print_statement(void) {
    ASTNode *node = create_node(AST_PRINT);
    advance(); // consume 'print'
    node->left = parse_expression();
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        exit(1);
    }
    advance();
    return node;
}

// Parse a block: { statement1; statement2; ... }
static ASTNode *parse_block(void) {
    expect(TOKEN_LBRACE); // consume '{'
    ASTNode *block_node = create_node(AST_BLOCK);
    ASTNode *current = NULL;
    while (!match(TOKEN_RBRACE) && !match(TOKEN_EOF)) {
        ASTNode *stmt = parse_statement();
        if (block_node->left == NULL) {
            block_node->left = stmt;
            current = block_node->left;
        } else {
            current->right = stmt;
            current = current->right;
        }
    }
    if (!match(TOKEN_RBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK, current_token);
        exit(1);
    }
    expect(TOKEN_RBRACE); // consume '}'
    return block_node;
}

// Parse factorial function call: factorial(expression)
static ASTNode *parse_factorial(void) {
    ASTNode *node = create_node(AST_FUNCALL);
    node->token = current_token; // should be "factorial"
    advance(); // consume 'factorial'
    expect(TOKEN_LPAREN);
    node->left = parse_expression(); // argument in left child
    expect(TOKEN_RPAREN);
    return node;
}

// -----------------------------------------------------------------
// Top-Level Statement Parsing
// -----------------------------------------------------------------
static ASTNode *parse_statement(void) {
    if (match(TOKEN_INT)) {
        return parse_declaration();
    } else if (match(TOKEN_IDENTIFIER)) {
        // Could be an assignment (function calls are handled in parse_primary).
        return parse_assignment();
    } else if (match(TOKEN_IF)) {
        return parse_if_statement();
    } else if (match(TOKEN_WHILE)) {
        return parse_while_statement();
    } else if (match(TOKEN_REPEAT)) {
        return parse_repeat_statement();
    } else if (match(TOKEN_PRINT)) {
        return parse_print_statement();
    } else if (match(TOKEN_LBRACE)) {
        return parse_block();
    }
    printf("Syntax Error: Unexpected token '%s' @ \n", current_token.lexeme);
    exit(1);
}

// Parse a program: a sequence of statements.
static ASTNode *parse_program(void) {
    ASTNode *program = create_node(AST_PROGRAM);
    ASTNode *current = program;
    while (!match(TOKEN_EOF)) {
        current->left = parse_statement();
        if (!match(TOKEN_EOF)) {
            current->right = create_node(AST_PROGRAM);
            current = current->right;
        }
    }
    return program;
}

// -----------------------------------------------------------------
// Parser Initialization and Main Parse Function
// -----------------------------------------------------------------
void parser_init(const char *input) {
    source = input;
    position = 0;
    advance(); // get first token
}

ASTNode *parse(void) {
    return parse_program();
}

// -----------------------------------------------------------------
// AST Debug Printing and Memory Cleanup
// -----------------------------------------------------------------
void print_ast(ASTNode *node, int level) {
    if (!node) return;
    for (int i = 0; i < level; i++) printf("  ");
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            break;
        case AST_VARDECL:
            printf("VarDecl: %s\n", node->token.lexeme);
            break;
        case AST_ASSIGN:
            printf("Assign\n");
            break;
        case AST_NUMBER:
            printf("Number: %s\n", node->token.lexeme);
            break;
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->token.lexeme);
            break;
        case AST_BINOP:
            printf("BinaryOp: %s\n", node->token.lexeme);
            break;
        case AST_IF:
            printf("If\n");
            break;
        case AST_WHILE:
            printf("While\n");
            break;
        case AST_REPEAT:
            printf("Repeat-Until\n");
            break;
        case AST_PRINT:
            printf("Print\n");
            break;
        case AST_BLOCK:
            printf("Block\n");
            break;
        case AST_FUNCALL:
            printf("FuncCall: %s\n", node->token.lexeme);
            break;
        default:
            printf("Unknown node type\n");
    }
    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
}

void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

// -----------------------------------------------------------------
// Main Function for Testing
// -----------------------------------------------------------------
char* read_file(const char* filename) {
    FILE *file = fopen(filename, "rb");  // Open in binary mode
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size < 0) {  // Handle ftell failure
        fclose(file);
        perror("ftell failed");
        return NULL;
    }

    char *buffer = (char *)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        perror("Memory allocation failed");
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, size, file);
    buffer[bytesRead] = '\0';  // Use bytesRead, not size

    fclose(file);

    for (char* p = buffer; *p; ++p) {
        if (*p == '\r') {
            *p = ' ';  // Or simply remove it by shifting the text left
        }
    }

    return buffer;
}

int main() {
    const char *valid_filename = "test/input_valid.txt";
    const char *invalid_filename = "test/input_invalid.txt";

    printf("Parsing valid input from %s:\n", valid_filename);
    char *valid_input = read_file(valid_filename);
    if (valid_input) {
        printf("Parsing:\n%s\n", valid_input);
        parser_init(valid_input);
        ASTNode *ast = parse();
        if (ast) {
            printf("\nAbstract Syntax Tree for valid input:\n");
            print_ast(ast, 0);
            free_ast(ast);
        } else {
            printf("Error parsing valid input.\n");
        }
        free(valid_input);
    }

    printf("\nParsing invalid input from %s:\n", invalid_filename);
    char *invalid_input = read_file(invalid_filename);
    if (invalid_input) {
        printf("Parsing:\n%s\n", invalid_input);
        parser_init(invalid_input);
        ASTNode *ast = parse();
        if (!ast) {
            printf("Error parsing invalid input as expected.\n");
        } else {
            printf("\nAbstract Syntax Tree for invalid input (unexpected):\n");
            print_ast(ast, 0);
            free_ast(ast);
        }
        free(invalid_input);
    }

    return 0;
}
