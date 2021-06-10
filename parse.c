#include "9cc.h"

// token, node 関連の関数？
// と上記

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
bool consume(char *op) {
  if (token->kind != TK_RESERVED 
      || strlen(op) != token->len
      || memcmp(token->str,op,token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号か否か。
// 偽:error; <-> 真:トークンを1つ読み進める;
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) !=token->len 
      || memcmp(token->str,op,token->len))
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len=len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q){
  return memcmp(p,q,strlen(q))==0;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
  char *p=user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
  // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }
  // ２ 文字文の演算子の場合
    if(startswith(p,"==") || startswith(p,"!=")
        || startswith(p,"<=") || startswith(p,">=")){
      cur =new_token(TK_RESERVED,cur,p,2);
      p+=2;
      continue;
    }

  // 1 文字文の演算子の場合
    if(strchr("+-*/()<>",*p)){
      cur = new_token(TK_RESERVED, cur, p,1);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p,0);
      char *q=p;
      cur->val = strtol(p, &p, 10);
      cur->len=p-q;
      continue;
    }

    error_at(token->str,"トークナイズできません");
  }

  new_token(TK_EOF, cur, p,0);
  return head.next;
}

// 
// making Nodes
// 
// Nodekind が kind であるような新しいnodeを作成する
Node *new_node(Nodekind kind){
  Node *node=calloc(1,sizeof(Node));
  node->kind=kind;
  return node;
}

// 右辺と左辺をつなぐようなNKがkindであるような新しいnodeを作成する
Node *new_binary(Nodekind kind,Node *lhs,Node *rhs){
  Node *node=new_node(kind);
  node->lhs=lhs;
  node->rhs=rhs;
  return node;
}

// 整数型の終端記号であるような新しいnodeを作成する
Node *new_num(int val){
  Node *node=new_node(ND_NUM);
  node->val=val;
  return node;
}

// 
// RDP
// 
Node *expr(){
  return equality();
}

Node *equality(){
  Node *node=relational();

  for(;;){
    if(consume("=="))
      node=new_binary(ND_EQ,node,relational());
    else if(consume("!="))
      node=new_binary(ND_NE,node,relational());
    else return node;
  }
}

Node *relational(){
  Node *node=add();

  for(;;){
    if(consume("<="))
      node=new_binary(ND_LE,node,add());
    else if(consume(">="))
      node=new_binary(ND_LE,add(),node);
    else if(consume("<"))
      node=new_binary(ND_LT,node,add());
    else if(consume(">"))
      node=new_binary(ND_LT,add(),node);
    else return node;
  }
}

Node *add(){
  Node *node=mul();

  for(;;){
    if(consume("+"))
      node=new_binary(ND_ADD,node,mul());
    else if(consume("-"))
      node=new_binary(ND_SUB,node,mul());
    else return node;
  }
}

Node *mul(){
  Node *node=unary();
  for(;;){
    // consume:偽ならTokenはそのままなので繰り返し判定可能
    if(consume("*"))
      node=new_binary(ND_MUL,node,unary());
    else if(consume("/"))
      node=new_binary(ND_DIV,node,unary());
    else return node;
  }
}

Node *unary(){
  if(consume("+"))
    return primary();
  if(consume("-"))
    return new_binary(ND_SUB,new_num(0),primary());
  return primary();
}

Node *primary(){
  if(consume("(")){
    Node *node=expr();
    expect(")");
    return node;
  }

  return new_num(expect_number());
}


