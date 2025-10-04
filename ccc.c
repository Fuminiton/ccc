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

// Input program
char *user_input;

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


// Report the error position
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int position = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", position, " ");
  fprintf(stderr, "^ ");
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
    error_at(token->str, "expected '%c'", op);
  }
  token = token->next;
}

// If the next token is numeric, read one token.
// Otherwise, report an error.
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "expected a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// Create a new token and append it to cur.
Token *new_token(TokenKind kind, Token *current_token, char *str) {
  Token *new_token = calloc(1, sizeof(Token));
  new_token->kind = kind;
  new_token->str = str;
  current_token->next = new_token;
  return new_token;
}

// Tokenize the input string p and return it.
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *current_token = &head;

  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Punctuator
    if (*p == '+' || *p == '-') {
      current_token = new_token(TK_RESERVED, current_token, p++);
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      current_token = new_token(TK_NUM, current_token, p);
      // strtol automatically advances the pointer
      // to the address of the character after the number.
      current_token->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "expected a number");
  }
  new_token(TK_EOF, current_token, p);
  return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "The number of arguments is incorrect\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();

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
