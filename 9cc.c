#include "9cc.h"

// 入力プログラム
char *user_input;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    locals = calloc(1, sizeof(LVar));
    // トークナイズしてパースする
    user_input = argv[1];
//    fprintf(stderr, "input: %s \n", user_input);
    tokenize(user_input);
    Token* t = token;
//    for(;;){
//        fprintf(stderr, "tokenized:: str:%s, len:%d, kind:%d \n", t->str, t->len, t->kind);
//        t = t->next;
//        if(!t)
//            break;
//    }

    program();


    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // 先頭の式から順にコード生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        // 式の評価結果としてスタックに一つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}