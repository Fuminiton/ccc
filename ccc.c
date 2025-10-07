#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* struct / union / enum */
// Token Kinds
typedef enum {
  TK_RESERVED,     // Symbol
  TK_NUM,          // Integer token
  TK_EOF,          // End-of-input token
} TokenKind;

// AST Node Kinds
typedef enum {
  ND_ADD,          // +
  ND_SUB,          // -
  ND_MUL,          // *
  ND_DIV,          // /
  ND_EQ,           // ==
  ND_NE,           // !=
  ND_LT,           // <
  ND_LE,           // <=
  ND_NUM,          // number
} NodeKind;

/* typedef */
typedef struct Token Token;
typedef struct Node Node;

// Token Type
struct Token {
  TokenKind kind;  // Token struct
  Token *next;     // Next input token
  int val;         // Numeric value when kind is TK_NUM
  char *str;       // Token String
  int len;         // Token length
};

// Node Type
struct Node {
  NodeKind kind;   // Node struct
  Node *lhs;       // left-hand side
  Node *rhs;       // right-hand side
  int val;
};


/* global variables */
// Input program
char *user_input;

// Token currently under consideration
Token *token;


void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *current_token, char *str, int len);
bool startwith(char *p, char *q);
Token *tokenize();
Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_num(int val);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);


int main(int argc, char **argv) {
    if (argc != 2) {
        error("%s: Invalid number of arguments", argv[0]);
    }

    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    
    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}

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

// Create a new token and append it to cur.
Token *new_token(TokenKind kind, Token *current_token, char *str, int len) {
  Token *new_token = calloc(1, sizeof(Token));
  new_token->kind = kind;
  new_token->str = str;
  new_token->len = len;
  current_token->next = new_token;
  return new_token;
}

bool startwith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
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

    // Multi-letter punctuator
    if (startwith(p, "==") || startwith(p, "!=") ||
        startwith(p, "<=") || startwith(p, ">=")) {
      current_token = new_token(TK_RESERVED, current_token, p, 2);
      p += 2;
      continue;
    }

    // Single-letter punctuator
    if (strchr("+-*/()<>", *p)) {
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

    error_at(p, "invalid token");
  }
  new_token(TK_EOF, current_token, p, 0);
  return head.next;
}

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

// expr = equality
Node *expr() {
  return equality();
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_binary(ND_EQ, node, relational());
      continue;
    }
    if (consume("!=")) {
      node = new_binary(ND_NE, node, relational());
      continue;
    }
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<")) {
      node = new_binary(ND_LT, node, add());
      continue;
    }
    if (consume("<=")) {
      node = new_binary(ND_LE, node, add());
      continue;
    }
    if (consume(">")) {
      node = new_binary(ND_LT, add(), node);
      continue;
    }
    if (consume(">=")) {
      node = new_binary(ND_LE, add(), node);
      continue;
    }
    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_binary(ND_ADD, node, mul());
      continue;
    }
    if (consume("-")) {
      node = new_binary(ND_SUB, node, mul());
      continue;
    }
    return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_binary(ND_MUL, node, unary());
      continue;
    }
    if (consume("/")) {
      node = new_binary(ND_DIV, node, unary());
      continue;
    }
    return node;
  }
}

// unary = ("+" | "-")? unary
Node *unary() {
  if (consume("+")) {
    return unary();
  }
  if (consume("-")) {
    return new_binary(ND_SUB, new_num(0), unary());
  }
  return primary();
}

// primary = "(" expr ")" | num
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  return new_num(expect_number());
}

// 
// Code generator
//

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}
