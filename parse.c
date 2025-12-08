#include "ccc.h"


Node *code[100];


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


Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();



// program = stmt*
void program() {
  int i = 0;
  while (!at_eof()) {
    code[i] = stmt();
    i++;
  }
  code[i] = NULL;
}

// stmt = expr;
Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

// expr = assign
Node *expr() {
  return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_binary(ND_ASSIGN, node, assign());
  }
  return node;
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

  Token *t = consume_identifier();
  if (t) {
    Node *node = new_node(ND_LVAR);
    node->offset = (t->str[0] - 'a' + 1) * 8;
    return node;
  }

  return new_num(expect_number());
}
