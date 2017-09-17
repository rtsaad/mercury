#define NET 257
#define TR 258
#define PL 259
#define LB 260
#define PR 261
#define LBRACK 262
#define RBRACK 263
#define KP 264
#define MP 265
#define INFI 266
#define STOPW 267
#define INIB 268
#define ARROW 269
#define IDENT 270
#define NAT 271
typedef union{
 char stval[100];
 char* ptr; 
 int natural;
 NetParserType *net;
 DataHolder transition;
 DataHolder place;
 DataHolder label;
 WeightType *weight_value;
 ArcType arc;
 ListType *list;
 void * arc_pl;
 } YYSTYPE;
extern YYSTYPE yylval;
