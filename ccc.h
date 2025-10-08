#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
// tokenize.c
//
typedef enum {
  TK_RESERVED,     // Keyword or punctuator
  TK_NUM,          // Integer literal
  TK_EOF,          // End-of-file marker
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;  // Token struct
  Token *next;     // Next input token
  int val;         // Numeric value when kind is TK_NUM
  char *str;       // Token String
  int len;         // Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *current_token, char *str, int len);
Token *tokenize();

extern char *user_input;  // Input program
extern Token *token;      // Token currently under consideration


//
// parse.c
//
typedef enum {
  ND_ADD,          // +
  ND_SUB,          // -
  ND_MUL,          // *
  ND_DIV,          // /
  ND_EQ,           // ==
  ND_NE,           // !=
  ND_LT,           // <
  ND_LE,           // <=
  ND_NUM,          // Integer
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;   // Node kind
  Node *lhs;       // Left-hand side
  Node *rhs;       // Right-hand side
  int val;         // Used if kind == ND_NUM
};

Node *expr();


// 
// codegen.c
// 
void codegen(Node *node);
