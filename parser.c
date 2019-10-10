//
// Created by kunihikot on 19/07/03.
//
#include "9cc.h"

// 現在着目しているトークン
Token *token;
Node *code[100];
LVar *locals;

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

Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node *expr() {
    return assign();
}

Node *stmt() {
    Node *node;
    if(consume_if()){
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        if (consume("(")) {
            node->cond = expr();
            expect(")");
            node->then = stmt();
            if(consume_else()){
                node->els = stmt();
            }
        }
        return node;
    }

    if(consume_while()){
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        if (consume("(")) {
            node->cond = expr();
            expect(")");
            node->lhs = stmt();
        }
        return  node;
    }

    if (consume_return()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else {
        node = expr();
    }
    expect(";");
    return node;
}


void program() {
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
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
    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }

        return node;
    }

    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    // そうでなければ数値のはず
    return new_node_num(expect_number());
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

bool consume_return(){
    if (token->kind != TK_RETURN){
        return false;
    }
    token = token->next;
    return true;
}

bool consume_if(){
    if (token->kind != TK_IF){
        return false;
    }
    token = token->next;
    return true;
}

bool consume_else(){
    if (token->kind != TK_ELSE){
        return false;
    }
    token = token->next;
    return true;
}


bool consume_for(){
    if (token->kind != TK_FOR){
        return false;
    }
    token = token->next;
    return true;
}

bool consume_while(){
    if (token->kind != TK_WHILE){
        return false;
    }
    token = token->next;
    return true;
}


Token *consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token *current = token;
    token = token->next;
    return current;
}


// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len) != 0)
        error("'%s'ではありません", op);
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
        tok->str = (char *) calloc(1, 2);
        strncpy(tok->str, str, 2);
    } else if (kind == TK_IDENT) {
        tok->len = strlen(str);
        tok->str = str;
    } else if(kind == TK_RETURN){
        tok->len = 5;
        tok->str = str;
    } else if(kind == TK_IF){
        tok->len = 2;
        tok->str = str;
    } else if(kind == TK_WHILE){
        tok->len = 5;
        tok->str = str;
    } else {
        tok->len = 1;
        tok->str = (char *) calloc(1, 1);
        strncpy(tok->str, str, 1);
    }
    cur->next = tok;

    return tok;
}

// 入力文字列pをトークナイズしてそれを返す
void tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p) {

        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        // 予約語判定
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == ')' || *p == '(' || *p == '<' || *p == '>' ||
            *p == '=' || *p == '!' || *p == ';') {
            cur = new_token(TK_RESERVED, cur, p++);
            if (cur->len == 2) {
                p++;
            }
            continue;
        }

        // 数値判定
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        // returnの判定
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, "return");
            p += 6;
            continue;
        }

        // ifの判定
        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, "if");
            p += 2;
            continue;
        }

        // elseの判定
        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_ELSE, cur, "else");
            p += 4;
            continue;
        }

        // whileの判定
        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_WHILE, cur, "while");
            p += 5;
            continue;
        }

        // ローカル変数
        if ('a' <= *p && *p <= 'z') {
            int len = 1;
            char *start = p;
            for (;;) {
                p++;
                if ('a' <= *p && *p <= 'z') {
                    len++;
                } else {
                    break;
                }
            }
            char *dst = (char *) malloc(sizeof(char) * sizeof(len));
            strncpy(dst, start, len);
            dst[len] = '\0';
            cur = new_token(TK_IDENT, cur, dst);
            continue;
        }


        error("トークナイズできません");
    }
    new_token(TK_EOF, cur, p);
    token = head.next;
}


// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}