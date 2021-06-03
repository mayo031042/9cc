#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 
// Tokenizer
// 

typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// token type
typedef struct Token Token;
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
};

char *user_input;
// 現在着目しているトークン
Token *token;

// 
// error
// 

void error(char *fmt,...){
  va_list ap;
  va_start(ap,fmt);
  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
  exit(1);
}

void error_at(char *loc,char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos=loc-user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号か否か。
// 偽:ret 0; <-> 真:トークンを1つ読み進めてret 1;
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号か否か。
// 偽:error; <-> 真:トークンを1つ読み進める;
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
  // error_at -> exit(1)
    error_at(token->str,"'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが期待している記号か否か。
// 偽:error; <-> 真:トークンを1つ読み進めて ret val;
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str,"数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
// 関数ポインタではなくポインタを返す関数なので（）は不要
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
  // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if(strchr("+-*/()",*p)){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(token->str,"トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;

}

// 
// RDP
// 

typedef enum{
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
}Nodekind;

typedef struct Node Node;
// 抽象構文木のノードの型
struct Node{
  Nodekind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

Node *new_node(Nodekind kind){
  Node *node=calloc(1,sizeof(Node));
  node->kind=kind;
  return node;
}

Node *new_binary(Nodekind kind,Node *lhs,Node *rhs){
  Node *node=new_node(kind);
  node->lhs=lhs;
  node->rhs=rhs;
  return node;
}

Node *new_num(int val){
  Node *node=new_node(ND_NUM);
  node->val=val;
  return node;
}

// expr = mul("+"mul | "-"mul)*
Node *expr();
// mul = primary("*"primary | "/"primary)*
Node *mul();
// primary = "("expr")" | num
Node *primary();

// expr() -> mul() -> primary()
Node *expr(){
  Node *node=mul();
  for(;;){
    // consume:偽ならTokeはそのままなので繰り返し判定可能
    if(consume('+'))
      node=new_binary(ND_ADD,node,mul());
    else if(consume('-'))
      node=new_binary(ND_SUB,node,mul());
    else return node;
  }
}

Node *mul(){
  Node *node=primary();
  for(;;){
    // consume:偽ならTokeはそのままなので繰り返し判定可能
    if(consume('*'))
      node=new_binary(ND_MUL,node,primary());
    else if(consume('/'))
      node=new_binary(ND_DIV,node,primary());
    else return node;
  }
}

Node *primary(){
  if(consume('(')){
    Node *node=expr();
    expect(')');
    return node;
  }

  return new_num(expect_number());
}

// 
// code generator
// 

void gen(Node *node){
  if(node->kind==ND_NUM){
    printf("  push %d\n",node->val);
    return ;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind){
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
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2)
    error_at(token->str,"引数の個数が正しくありません");

  user_input=argv[1];
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