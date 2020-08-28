//
// Created by kunihikot on 19/07/03.
//

#include "9cc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}


void gen(Node *node) {

    if (node->kind == ND_BLOCK) {
        for (Node *n = node->body; n; n = n->next)
            gen(n);
        return;
    }

    if (node->kind == ND_RETURN) {
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    }

    if(node->kind == ND_IF){
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if(node->els){
            printf("  je  .LelseXXX\n");
            gen(node->then);
            printf("  jmp .LendXXX\n");
            printf(".LelseXXX:\n");
            gen(node->els);
            printf(".LendXXX:\n");
        }else{
            printf("  je  .LendXXX\n");
            gen(node->then);
            printf(".LendXXX:\n");
        }
        return;
    }

    if(node->kind == ND_WHILE){
        printf(".LbeginXXX:\n");
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .LendXXX\n");
        gen(node->lhs);
        printf("  jmp .LbeginXXX\n");
        printf(".LendXXX:\n");
        return;
    }


    if(node->kind == ND_FOR){
        gen(node->init);
        printf(".LbeginXXX:\n");
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .LendXXX\n");
        gen(node->lhs);
        gen(node->step);
        printf("  jmp .LbeginXXX\n");
        printf(".LendXXX:\n");
        return;
    }


    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
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
        case ND_EQUAL:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NOT_EQUAL:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LESS_THAN:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LESS_THAN_EQUAL:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_GREATER_THAN:
            printf("  cmp rdi, rax\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_GREATER_THAN_EQUAL:
            printf("  cmp rdi, rax\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
    }
    printf("  push rax\n");
}
