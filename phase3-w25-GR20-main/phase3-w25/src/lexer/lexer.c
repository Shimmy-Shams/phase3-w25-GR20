#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../../include/tokens.h"
#include "../../include/lexer.h"

static int current_line = 1;

// Keywords table
static struct {
    const char* word;
    TokenType type;
} keywords[] = {
    {"if", TOKEN_IF},
    {"while", TOKEN_WHILE},
    {"repeat", TOKEN_REPEAT},
    {"until", TOKEN_UNTIL},
    {"int", TOKEN_INT},
    {"print", TOKEN_PRINT}
};

static int is_keyword(const char* word) {
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i].word) == 0) {
            return keywords[i].type;
        }
    }
    return 0;
}

void print_error(ErrorType error, int line, const char* lexeme) {
    printf("Lexical Error at line %d: ", line);
    switch(error) {
        case ERROR_INVALID_CHAR:
            printf("Invalid character '%s'\n", lexeme);
            break;
        case ERROR_INVALID_NUMBER:
            printf("Invalid number format\n");
            break;
        case ERROR_CONSECUTIVE_OPERATORS:
            printf("Consecutive operators not allowed\n");
            break;
        case ERROR_INVALID_IDENTIFIER:
            printf("Invalid identifier\n");
            break;
        case ERROR_UNEXPECTED_TOKEN:
            printf("Unexpected token '%s'\n", lexeme);
            break;
        default:
            printf("Unknown error\n");
    }
}

void print_token(Token token) {
    if (token.error != ERROR_NONE) {
        print_error(token.error, token.line, token.lexeme);
        return;
    }
    printf("Token: ");
    switch(token.type) {
        case TOKEN_NUMBER:     printf("NUMBER"); break;
        case TOKEN_OPERATOR:   printf("OPERATOR"); break;
        case TOKEN_IDENTIFIER: printf("IDENTIFIER"); break;
        case TOKEN_EQUALS:     printf("EQUALS"); break;
        case TOKEN_SEMICOLON:  printf("SEMICOLON"); break;
        case TOKEN_LPAREN:     printf("LPAREN"); break;
        case TOKEN_RPAREN:     printf("RPAREN"); break;
        case TOKEN_LBRACE:     printf("LBRACE"); break;
        case TOKEN_RBRACE:     printf("RBRACE"); break;
        case TOKEN_IF:         printf("IF"); break;
        case TOKEN_INT:        printf("INT"); break;
        case TOKEN_PRINT:      printf("PRINT"); break;
        case TOKEN_WHILE:      printf("WHILE"); break;
        case TOKEN_REPEAT:     printf("REPEAT"); break;
        case TOKEN_UNTIL:      printf("UNTIL"); break;
        case TOKEN_EOF:        printf("EOF"); break;
        default:               printf("UNKNOWN");
    }
    printf(" | Lexeme: '%s' | Line: %d\n", token.lexeme, token.line);
}

Token get_next_token(const char* input, int* pos) {
    Token token = {TOKEN_ERROR, "", current_line, ERROR_NONE};
    char c;
    
    // Skip whitespace and update line count.
    // Also, skip over block comments "/* ... */"
    while (1) {
        // Skip whitespace
        while ((c = input[*pos]) != '\0' && (c == ' ' || c == '\n' || c == '\t')) {
            if (c == '\n') {
                current_line++;
            }
            (*pos)++;
        }
        // Check for block comment start "/*"
        if (input[*pos] == '/' && input[*pos + 1] == '*') {
            (*pos) += 2;  // Skip "/*"
            while (input[*pos] != '\0' && !(input[*pos] == '*' && input[*pos + 1] == '/')) {
                if (input[*pos] == '\n') {
                    current_line++;
                }
                (*pos)++;
            }
            if (input[*pos] != '\0') {
                (*pos) += 2;  // Skip closing "*/"
            }
            continue;  // After skipping the comment, start over.
        }
        break;
    }
    
    if (input[*pos] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        current_line = 0;
        return token;
    }
    
    c = input[*pos];
    
    // Handle numbers
    if (isdigit(c)) {
        int i = 0;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while (isdigit(c) && i < sizeof(token.lexeme) - 1);
        token.lexeme[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }
    
    // Handle identifiers and keywords
    if (isalpha(c) || c == '_') {
        int i = 0;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while ((isalnum(c) || c == '_') && i < sizeof(token.lexeme) - 1);
        token.lexeme[i] = '\0';
        // Check if it's a keyword
        TokenType keyword_type = is_keyword(token.lexeme);
        if (keyword_type) {
            token.type = keyword_type;
        } else {
            token.type = TOKEN_IDENTIFIER;
        }
        return token;
    }
    
    // Handle operators and delimiters
    (*pos)++;
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    switch(c) {
        case '+': case '-': case '*': case '/':
            token.type = TOKEN_OPERATOR;
            break;
        case '>':
            if (input[*pos] == '=') {
                (*pos)++;
                strcpy(token.lexeme, ">=");
            }
            token.type = TOKEN_OPERATOR;
            break;
        case '<':
            if (input[*pos] == '=') {
                (*pos)++;
                strcpy(token.lexeme, "<=");
            }
            token.type = TOKEN_OPERATOR;
            break;
        case '=':
            if (input[*pos] == '=') {
                (*pos)++;
                strcpy(token.lexeme, "==");
                token.type = TOKEN_OPERATOR;
            } else {
                token.type = TOKEN_EQUALS;
            }
            break;
        case ';':
            token.type = TOKEN_SEMICOLON;
            break;
        case '(':
            token.type = TOKEN_LPAREN;
            break;
        case ')':
            token.type = TOKEN_RPAREN;
            break;
        case '{':
            token.type = TOKEN_LBRACE;
            break;
        case '}':
            token.type = TOKEN_RBRACE;
            break;
        default:
            token.error = ERROR_INVALID_CHAR;
            break;
    }
    return token;
}
