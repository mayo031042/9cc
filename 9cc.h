#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 関数、変数の宣言と列挙型、構造体の表記のみ
// これらは他ファイルをコンパイルするときに必要な情報のまとめ

// error
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

// Tokenize:
typedef enum
{
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;
struct Token
{
  TokenKind kind;
  Token *next;
  int val;   // kindがTK_NUMの場合、その数値
  char *str; // トークン文字列
  int len;
};

// Node:
typedef enum
{
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // < less than
  ND_LE,     // <= less than or equal to
  ND_ASSIGN, // ＝
  ND_LVAR,   //ローカル変数
  ND_NUM,    // 整数
} Nodekind;

typedef struct Node Node;
struct Node
{
  Nodekind kind;
  Node *lhs;
  Node *rhs;
  int val;
  int offset;
};

// extern
extern char *user_input;
extern Token *token;

// Tokenize -> func:
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();

Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();

// Node -> func:
Node *new_node(Nodekind kind);
Node *new_binary(Nodekind kind, Node *lhs, Node *rhs);
Node *new_num(int val);

//
// RDP
//
// prog = stmt*
// stmt = expr ";"
// expr = assign
// assign = equal("=" assign)?
// equal = rela("=="rela | "!="rela)*
// rela = add("<"add | "<="add | ">"add | ">="add)*
// add = mul("+"mul | "-"mul)*
// mul = unary("*"unary | "/"unary)*
// unary = ("+" | "-")? prim
// prim = num | ident | "("expr")"
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *ident();

void gen(Node *node);