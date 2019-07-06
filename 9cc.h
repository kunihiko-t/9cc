#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error_at(char *loc, char *fmt, ...);

extern char *user_input;

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
};

extern LVar *locals;

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
    ND_ASSIGN, // =
    ND_LVAR,   // ローカル変数
} NodeKind;


// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Node Node;


// 抽象構文木のノード
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // kindがND_NUMの場合のみ使う
    int offset;    // kindがND_LVARの場合のみ使う
};

extern Node *code[100];


//program    = stmt*
//stmt       = expr ";"
//expr       = assign
//assign     = equality ("=" assign)?
//equality   = relational ("==" relational | "!=" relational)*
//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
//add        = mul ("+" mul | "-" mul)*
//mul        = unary ("*" unary | "/" unary)*
//unary      = ("+" | "-")? term
//term       = num | ident | "(" expr ")"

void program();

Node *stmt();

Node *assign();

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

void gen_lval(Node *node);


Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

Node *new_node_num(int val);

typedef struct Token Token;


// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};

LVar *find_lvar(Token *tok);

bool consume(char *op);

Token *consume_ident();

extern Token *token;

void error(char *fmt, ...);

bool at_eof();

Token *new_token(TokenKind kind, Token *cur, char *str);

void tokenize(char *p);

