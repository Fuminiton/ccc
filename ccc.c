#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Token Kinds
typedef enum {
  TK_RESERVED,     // Symbol
  TK_NUM,          // Integer token
  TK_EOF,          // End-of-input token
} TokenKind;

typedef struct Token Token;

// Token Type
struct Token {
  TokenKind kind;  // Token type
  Token *next;     // Next input token
  int val;         // Numeric value when kind is TK_NUM
  char *str;       // Token String
};

// Token currently under consideration
Token *token;


// Function to report an error
// Takes the same arguments as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// If the next token matches the expected symbol,
// read one token and return true.
// Otherwise, return false.
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

// If the next token matches the expected symbol,
// read one token.
// Otherwise, report an error.
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error("'%c' is a unexpected symbol.", op);
  }
  token = token->next;
}

// If the next token is numeric, read one token.
// Otherwise, report an error.
int expect_number() {
  if (token->kind != TK_NUM) {
    error("Current token is not numeric.");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// Create a new token and append it to cur.
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// Tokenize the input string p and return it.
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      // strtol automatically advances the pointer
      // to the address of the character after the number.
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error("Can't tokenize.");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "The number of arguments is incorrect\n");
        return 1;
    }

    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    
    printf("  mov rax, %d\n", expect_number());

    while (!at_eof()) {
      if (consume('+')) {
        printf("  add rax, %d\n", expect_number());
        continue;
      }
      expect('-');
      printf("  sub rax, %d\n", expect_number());
    }

    printf("  ret\n");
    return 0;
}
