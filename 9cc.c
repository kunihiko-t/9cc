#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 入力プログラム
char *user_input;

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 抽象構文木のノードの種類
typedef enum {
    ND_EQUAL, // ==
    ND_NOT_EQUAL, // !=
    ND_GREATER_THAN_EQUAL, // >=
    ND_GREATER_THAN, // >
    ND_LESS_THAN_EQUAL, // <=
    ND_LESS_THAN, // <
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
} NodeKind;

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;


typedef struct Node Node;

bool consume(char *op);


//expr       = equality
//equality   = relational ("==" relational | "!=" relational)*
//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
//add        = mul ("+" mul | "-" mul)*
//mul        = unary ("*" unary | "/" unary)*
//unary      = ("+" | "-")? term
//term       = num | "(" expr ")"

Node *equality();

Node *relational();

Node *add();

Node *expr();

Node *mul();

Node *term();

Node *unary();

void expect(char *op);

int expect_number();

void gen(Node *node);

// 抽象構文木のノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // kindがND_NUMの場合のみ使う
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}


Node *expr() {
    return equality();
}

Node *equality() {
    Node *node = relational();
    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQUAL, node, relational());
        else if (consume("!="))
            node = new_node(ND_NOT_EQUAL, node, relational());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();
    for (;;) {
        if (consume(">="))
            node = new_node(ND_GREATER_THAN_EQUAL, node, add());
        else if (consume("<="))
            node = new_node(ND_LESS_THAN_EQUAL, node, add());
        else if (consume(">"))
            node = new_node(ND_GREATER_THAN, node, add());
        else if (consume("<"))
            node = new_node(ND_LESS_THAN, node, add());
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();
    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}


Node *unary() {
    if (consume("+"))
        return term();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), term());
    return term();
}


Node *term() {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    // そうでなければ数値のはず
    return new_node_num(expect_number());
}


typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};
// 現在着目しているトークン
Token *token;

/// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len) != 0)
        error("'%c'ではありません", op);
    token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM)
        error("数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    if (
            strlen(str) >= 2 &&
            (
                    strncmp(str, ">=", 2) == 0 ||
                    strncmp(str, "<=", 2) == 0 ||
                    strncmp(str, "==", 2) == 0 ||
                    strncmp(str, "!=", 2) == 0
            )
            ) {
        tok->len = 2;
        tok->str = (char*)calloc(1,2);
        strncpy(tok->str, str, 2);
    } else {
        tok->len = 1;
        tok->str = (char*)calloc(1,1);
        strncpy(tok->str, str, 1);
    }
    cur->next = tok;

    return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    fprintf(stderr, "p is %s \n", p);
    while (*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == ')' || *p == '(' || *p == '<' || *p == '>' ||
            *p == '=' || *p == '!') {
            cur = new_token(TK_RESERVED, cur, p++);
            if (cur->len == 2) {
                p++;
            }
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズできません");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}

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
    }

    printf("  push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    // トークナイズしてパースする
    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);
    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}