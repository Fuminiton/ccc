#include "ccc.h"


void gen_local_variable(Node *node) {
  if (node->kind != ND_LVAR) {
    error("a local variable required as left operand of assignment");
  }
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void codegen(Node *node) {
  static int label_count = 0;

  if (node->kind == ND_IF) {
    int c = label_count;
    label_count++;
    codegen(node->condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->else_clause) {
      printf("  je .L.else.%d\n", c);
      codegen(node->then_clause);
      printf("  jmp .L.end.%d\n", c);
      printf(".L.else.%d:\n", c);
      codegen(node->else_clause);
      printf(".L.end.%d:\n", c);
    }
    else {
      printf("  je .L.end.%d\n", c);
      codegen(node->then_clause);
      printf(".L.end.%d:\n", c);
    }
    return ;
  }
  if (node->kind == ND_WHILE) {
    int c = label_count;
    label_count++;
    printf(".L.begin.%d:\n", c);
    codegen(node->condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .L.end.%d\n", c);
    codegen(node->then_clause);
    printf("  jmp .L.begin.%d\n", c);
    printf(".L.end.%d:\n", c);
    return ;
  }
  if (node->kind == ND_RETURN) {
    codegen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return ;
  }
  if (node->kind == ND_ASSIGN) {
    gen_local_variable(node->lhs);
    codegen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return ;
  }
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return ;
  }
  if (node->kind == ND_LVAR) {
    gen_local_variable(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return ;
  }
  codegen(node->lhs);
  codegen(node->rhs);

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
      printf("  movzx rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzx rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzx rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzx rax, al\n");
      break;
  }

  printf("  push rax\n");
}

void gen() {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        codegen(code[i]);
        printf("  pop rax\n");
    }

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}
