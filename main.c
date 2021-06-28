#include "9cc.h"

// "=="が複数個って？

// 唯一の定義
Token *token;
char *user_input;

int main(int argc, char **argv)
{
  if (argc != 2)
    error_at(token->str, "引数の個数が正しくありません");

  user_input = argv[1];
  token = tokenize(user_input);

  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  gen(node);
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
// commit from funclocalvar