#include "ccc.h"


char *user_input;
Token *token;

// Report an error and exit.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
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
bool consume(char *op) {
  if (token->kind != TK_RESERVED) {
    return false;
  }
  if (strlen(op) != token->len) {
    return false;
  }
  if (memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

Token *consume_identifier() {
  if (token->kind != TK_IDENTIFER) {
    return NULL;
  }
  Token *t = token;
  token = token->next;
  return t;
}


// If the next token matches the expected symbol,

// read one token.
// Otherwise, report an error.
void expect(char *op) {
  if (token->kind != TK_RESERVED) {
    error_at(token->str, "expected '%c'", op);
  }
  if (strlen(op) != token->len) {
    error_at(token->str, "expected '%c'", op);
  }
  if (memcmp(token->str, op, token->len)) {
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

bool startwith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

bool is_local_variable(char *p) {
  return ('a' <= *p && *p <= 'z') || (*p == '_');
}

// Create a new token and append it to cur.

Token *new_token(TokenKind kind, Token *current_token, char *str, int len) {
  Token *new_token = calloc(1, sizeof(Token));
  new_token->kind = kind;
  new_token->str = str;
  new_token->len = len;
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

    if (strncmp(p, "return", 6) == 0 && !isalnum(p[6])) {
      current_token = new_token(TK_RESERVED, current_token, p, 6);
      p += 6;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !isalnum(p[2])) {
      current_token = new_token(TK_RESERVED, current_token, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !isalnum(p[4])) {
      current_token = new_token(TK_RESERVED, current_token, p, 4);
      p += 4;
      continue;
    }

    if (strncmp(p, "while", 5) == 0 && !isalnum(p[5])) {
      current_token = new_token(TK_RESERVED, current_token, p, 5);
      p += 5;
      continue;
    }

    // Multi-letter punctuator
    if (startwith(p, "==") || startwith(p, "!=") ||
        startwith(p, "<=") || startwith(p, ">=")) {
      current_token = new_token(TK_RESERVED, current_token, p, 2);
      p += 2;
      continue;
    }

    // Single-letter punctuator
    if (strchr("+-*/()<>;=", *p)) {
      current_token = new_token(TK_RESERVED, current_token, p++, 1);
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      current_token = new_token(TK_NUM, current_token, p, 0);
      char *q = p;
      // strtol automatically advances the pointer
      // to the address of the character after the number.
      current_token->val = strtol(p, &p, 10);
      current_token->len = p - q;
      continue;
    }

    // Local variable
    if (is_local_variable(p)) {
      char *start = p;
      while (is_local_variable(p)) {
        p++;
      }
      current_token = new_token(TK_IDENTIFER, current_token, start, p - start);
      continue;
    }

    error_at(p, "invalid token");
  }
  new_token(TK_EOF, current_token, p, 0);
  return head.next;
}
