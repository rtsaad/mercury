#define GLOBAL 257
#define EXISTENTIAL 258
#define UNTIL 259
#define LEADSTO 260
#define ALWAYS 261
#define EVENTUALLY 262
#define NEXT 263
#define NEGATION 264
#define RELEASE 265
#define IMPLIES 266
#define AND 267
#define OR 268
#define SMALLER 269
#define SMALLER_EQ 270
#define EQ 271
#define GREATER_EQ 272
#define GREATER 273
#define PLUS 274
#define PROD 275
#define TRUE_LEX 276
#define FALSE_LEX 277
#define DEADLOCK 278
#define LPAREN 279
#define RPAREN 280
#define IDENT 281
#define NAT 282
typedef union{
 char stval[100];
 char* ptr; 
 int natural;
 Formula * logic_formula;
 Expression * logic_expression;
 } YYSTYPE;
extern YYSTYPE logicslval;
