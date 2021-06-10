#include "9cc.h"

// "=="が複数個って？

// 現在着目しているトークン
Token *token;
char *user_input;

// main関数と上記のみ
int main(int argc, char **argv) {
	user_input=argv[1];

  if (argc != 2)
    error_at(token->str,"引数の個数が正しくありません");

  token = tokenize(user_input);
  Node *node=expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}