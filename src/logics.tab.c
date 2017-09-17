#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20050813

#define YYEMPTY (-1)
#define yyclearin    (yychar = YYEMPTY)
#define yyerrok      (yyerrflag = 0)
#define YYRECOVERING (yyerrflag != 0)

extern int yyparse(void);

static int yygrowstack(void);
#define yyparse logicsparse
#define yylex logicslex
#define yyerror logicserror
#define yychar logicschar
#define yyval logicsval
#define yylval logicslval
#define yydebug logicsdebug
#define yynerrs logicsnerrs
#define yyerrflag logicserrflag
#define yyss logicsss
#define yyssp logicsssp
#define yyvs logicsvs
#define yyvsp logicsvsp
#define yylhs logicslhs
#define yylen logicslen
#define yydefred logicsdefred
#define yydgoto logicsdgoto
#define yysindex logicssindex
#define yyrindex logicsrindex
#define yygindex logicsgindex
#define yytable logicstable
#define yycheck logicscheck
#define yyname logicsname
#define yyrule logicsrule
#define YYPREFIX "logics"
#line 1 "logics.y"
         
  #include "logics_struct.h"
  #include "petri_net.h"
  extern int  logicslineno;
  extern char logicstext[];
  void logicserror(char* msg);
  int logicslex(void);
  int sym[26];
  int logics_noerror=1;
  /*extern void /*<Formula>* / *FORMULA_MC;*/
  extern Net *parsed_net;
#line 13 "logics.y"
typedef union{
 char stval[100];
 char* ptr; 
 int natural;
 Formula * logic_formula;
 Expression * logic_expression;
 } YYSTYPE;
#line 66 "logics.tab.c"
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
#define YYERRCODE 256
short logicslhs[] = {                                        -1,
    0,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    3,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    2,    2,
    2,    2,    2,    2,
};
short logicslen[] = {                                         2,
    1,    1,    2,    3,    3,    4,    4,    3,    3,    3,
    3,    3,    1,    1,    3,    2,    3,    3,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    1,    3,
    3,    3,    3,    3,
};
short logicsdefred[] = {                                      0,
    0,    0,    0,   14,    0,    0,   39,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   40,   15,    0,    0,   42,   44,    0,    0,   22,    0,
   25,    0,   37,    0,   33,    0,   29,   41,   43,    0,
    0,    0,   19,    0,   23,    0,   35,    0,   31,    0,
   27,    0,    0,    0,
};
short logicsdgoto[] = {                                       8,
    9,   10,   11,
};
short logicssindex[] = {                                   -183,
 -120,  -88, -183,    0, -254, -261,    0,    0, -260, -140,
  -71, -183, -183, -183, -230, -183, -183, -183, -188, -260,
  -71, -254, -187,  -85, -179, -179, -183, -254, -254, -254,
 -254, -254, -179, -179, -254, -254, -254, -254, -254, -254,
 -254, -260, -260, -260, -183, -260, -260, -260, -183,  -71,
    0,    0, -179, -261,    0,    0, -260, -197,    0, -197,
    0, -197,    0, -197,    0, -197,    0,    0,    0, -213,
 -159, -197,    0, -197,    0, -197,    0, -197,    0, -197,
    0, -260, -260, -263,
};
short logicsrindex[] = {                                      0,
    0,    0,    0,    0,    0,    1,    0,    0,    2,    0,
    3,    0,    0,    0,    0,    0,    0,    0,    0,    5,
    3,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    6,    7,    8,    0,   18,   19,   20,    0,  155,
    0,    0,    0,    0,    0,    0,   21,   16,    0,   31,
    0,   46,    0,   61,    0,   76,    0,    0,    0,  166,
  168,   91,    0,  106,    0,  121,    0,  136,    0,  151,
    0,   23,   33,    0,
};
short logicsgindex[] = {                                      0,
   52,   10,   87,
};
#define YYTABLESIZE 448
short logicstable[] = {                                      27,
   13,    1,    2,    0,    3,    8,    9,    4,    0,   22,
   33,   34,   25,   26,   23,   20,   51,   10,   11,    5,
   12,    0,    6,    4,    5,    0,    6,    7,   45,   27,
   26,    0,    7,    0,   55,   56,    0,   58,   60,   62,
   64,   66,   68,   69,    0,   38,   72,   74,   76,   78,
   80,    0,   15,   19,   20,   37,   38,   39,   40,   41,
   34,    0,   84,   42,   43,   44,    0,   46,   47,   48,
   49,   27,    0,    1,    2,   30,   33,   34,   57,    0,
    3,   28,   29,   30,   31,   32,   33,   34,    0,   21,
   21,   24,   51,    0,    4,    5,   82,    6,    7,   53,
   83,   54,    7,    0,    0,   24,    0,   35,   50,   37,
   38,   39,   40,   41,   59,   61,   63,   65,   67,    0,
   36,   70,   71,   73,   75,   77,   79,   81,   28,   29,
   30,   31,   32,   33,   34,   32,    1,    2,    0,    0,
   12,   13,   14,    3,    0,    0,    0,    0,    0,    0,
   28,    0,    0,    0,   16,    0,    0,    4,    5,    0,
    6,    7,    0,    0,    0,   17,    0,   18,    1,    2,
    0,    0,   16,   17,   18,    3,    0,    0,    0,    0,
    0,   35,   36,   37,   38,   39,   40,   41,    0,    4,
    5,    0,    6,    7,   52,   35,   36,   37,   38,   39,
   40,   41,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   13,
   13,    2,    2,    3,    8,    9,    4,   13,   13,   13,
   13,   13,   13,   13,   20,   20,   10,   11,    5,   12,
   13,    6,   20,   20,   20,   20,   20,   20,   20,   26,
   26,    7,    0,    0,    0,   20,    0,   26,   26,   26,
   26,   26,   26,   26,   38,   38,    0,    0,    0,    0,
   26,    0,   38,   38,   38,   38,   38,   38,   38,   34,
   34,    0,    0,    0,    0,   38,    0,   34,   34,   34,
   34,   34,   34,   34,   30,   30,    0,    0,    0,    0,
   34,    0,   30,   30,   30,   30,   30,   30,   30,   21,
   21,    0,    0,    0,    0,   30,    0,   21,   21,   21,
   21,   21,   21,   21,   24,   24,    0,    0,    0,    0,
   21,    0,   24,   24,   24,   24,   24,   24,   24,   36,
   36,    0,    0,    0,    0,   24,    0,   36,   36,   36,
   36,   36,   36,   36,   32,   32,    0,    0,    0,    0,
   36,    0,   32,   32,   32,   32,   32,   32,   32,   28,
   28,    0,    0,   16,   16,   32,    0,   28,   28,   28,
   28,   28,   28,   28,   17,   17,   18,   18,    0,    0,
   28,    0,   17,   17,   16,   18,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   17,    0,   18,
};
short logicscheck[] = {                                     260,
    0,    0,    0,   -1,    0,    0,    0,    0,   -1,  264,
  274,  275,  274,  275,    5,    0,  280,    0,    0,    0,
    0,   -1,    0,  278,  279,   -1,  281,  282,  259,  260,
    0,   -1,    0,   -1,   25,   26,   -1,   28,   29,   30,
   31,   32,   33,   34,   -1,    0,   37,   38,   39,   40,
   41,   -1,    1,    2,    3,  269,  270,  271,  272,  273,
    0,   -1,   53,   12,   13,   14,   -1,   16,   17,   18,
  259,  260,   -1,  257,  258,    0,  274,  275,   27,   -1,
  264,  269,  270,  271,  272,  273,  274,  275,   -1,    3,
    0,    5,  280,   -1,  278,  279,   45,  281,  282,  279,
   49,  281,  282,   -1,   -1,    0,   -1,  267,   22,  269,
  270,  271,  272,  273,   28,   29,   30,   31,   32,   -1,
    0,   35,   36,   37,   38,   39,   40,   41,  269,  270,
  271,  272,  273,  274,  275,    0,  257,  258,   -1,   -1,
  261,  262,  263,  264,   -1,   -1,   -1,   -1,   -1,   -1,
    0,   -1,   -1,   -1,    0,   -1,   -1,  278,  279,   -1,
  281,  282,   -1,   -1,   -1,    0,   -1,    0,  257,  258,
   -1,   -1,  261,  262,  263,  264,   -1,   -1,   -1,   -1,
   -1,  267,  268,  269,  270,  271,  272,  273,   -1,  278,
  279,   -1,  281,  282,  280,  267,  268,  269,  270,  271,
  272,  273,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  259,
  260,  259,  260,  259,  259,  259,  259,  267,  268,  269,
  270,  271,  272,  273,  259,  260,  259,  259,  259,  259,
  280,  259,  267,  268,  269,  270,  271,  272,  273,  259,
  260,  259,   -1,   -1,   -1,  280,   -1,  267,  268,  269,
  270,  271,  272,  273,  259,  260,   -1,   -1,   -1,   -1,
  280,   -1,  267,  268,  269,  270,  271,  272,  273,  259,
  260,   -1,   -1,   -1,   -1,  280,   -1,  267,  268,  269,
  270,  271,  272,  273,  259,  260,   -1,   -1,   -1,   -1,
  280,   -1,  267,  268,  269,  270,  271,  272,  273,  259,
  260,   -1,   -1,   -1,   -1,  280,   -1,  267,  268,  269,
  270,  271,  272,  273,  259,  260,   -1,   -1,   -1,   -1,
  280,   -1,  267,  268,  269,  270,  271,  272,  273,  259,
  260,   -1,   -1,   -1,   -1,  280,   -1,  267,  268,  269,
  270,  271,  272,  273,  259,  260,   -1,   -1,   -1,   -1,
  280,   -1,  267,  268,  269,  270,  271,  272,  273,  259,
  260,   -1,   -1,  259,  260,  280,   -1,  267,  268,  269,
  270,  271,  272,  273,  259,  260,  259,  260,   -1,   -1,
  280,   -1,  267,  268,  280,  268,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  280,   -1,  280,
};
#define YYFINAL 8
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 282
#if YYDEBUG
char *logicsname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"GLOBAL","EXISTENTIAL","UNTIL",
"LEADSTO","ALWAYS","EVENTUALLY","NEXT","NEGATION","RELEASE","IMPLIES","AND",
"OR","SMALLER","SMALLER_EQ","EQ","GREATER_EQ","GREATER","PLUS","PROD",
"TRUE_LEX","FALSE_LEX","DEADLOCK","LPAREN","RPAREN","IDENT","NAT",
};
char *logicsrule[] = {
"$accept : program",
"program : formula",
"formula : expbool",
"formula : NEGATION formula",
"formula : GLOBAL NEXT formula",
"formula : EXISTENTIAL NEXT formula",
"formula : GLOBAL formula UNTIL formula",
"formula : EXISTENTIAL formula UNTIL formula",
"formula : GLOBAL ALWAYS formula",
"formula : GLOBAL EVENTUALLY formula",
"formula : EXISTENTIAL ALWAYS formula",
"formula : EXISTENTIAL EVENTUALLY formula",
"formula : formula LEADSTO formula",
"expbool : IDENT",
"expbool : DEADLOCK",
"expbool : LPAREN expbool RPAREN",
"expbool : NEGATION expbool",
"expbool : expbool AND expbool",
"expbool : expbool OR expbool",
"expbool : expbool SMALLER expbool",
"expbool : expmath SMALLER expmath",
"expbool : expbool SMALLER expmath",
"expbool : expmath SMALLER expbool",
"expbool : expbool SMALLER_EQ expbool",
"expbool : expbool SMALLER_EQ expmath",
"expbool : expmath SMALLER_EQ expbool",
"expbool : expmath SMALLER_EQ expmath",
"expbool : expbool GREATER expbool",
"expbool : expbool GREATER expmath",
"expbool : expmath GREATER expbool",
"expbool : expmath GREATER expmath",
"expbool : expbool GREATER_EQ expbool",
"expbool : expbool GREATER_EQ expmath",
"expbool : expmath GREATER_EQ expbool",
"expbool : expmath GREATER_EQ expmath",
"expbool : expbool EQ expbool",
"expbool : expbool EQ expmath",
"expbool : expmath EQ expbool",
"expbool : expmath EQ expmath",
"expmath : NAT",
"expmath : LPAREN expmath RPAREN",
"expmath : expmath PLUS expmath",
"expmath : IDENT PLUS expmath",
"expmath : expmath PROD expmath",
"expmath : IDENT PROD expmath",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static int      yystacksize;
#line 136 "logics.y"

#include "reset_define_includes.h"
#define STDIOLIB
#define STDLIB
#define STRINGLIB
#include "standard_includes.h"

#include "flags.h"

extern char linebuf[256];

unboubd_variable(char *var){
  logics_noerror=0;
  printf("\n CLT PARSER: Unbound Variable %s \n", var);
  exit(EXIT_FAILURE);
}

void logicserror(char* msg)
{
 logics_noerror=0;
  printf("Line %d: %s at \" %s \"\n",logicslineno, msg, logicstext);

}
#line 376 "logics.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse(void)
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

#ifdef lint
    goto yyerrlab;
#endif

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 44 "logics.y"
{FORMULA_MC = yyvsp[0].logic_formula;}
break;
case 2:
#line 48 "logics.y"
{yyval.logic_formula = logic_create_formula_expression(yyvsp[0].logic_expression);}
break;
case 3:
#line 50 "logics.y"
{yyval.logic_formula = logic_create_formula_unary(yyvsp[0].logic_formula,L_NEGATION);}
break;
case 4:
#line 51 "logics.y"
{Formula *next = logic_create_formula_unary(yyvsp[0].logic_formula,L_NEXT);
                                              yyval.logic_formula = logic_create_formula_unary(next,L_GLOBAL);}
break;
case 5:
#line 53 "logics.y"
{Formula *next = logic_create_formula_unary(yyvsp[0].logic_formula,L_NEXT);
                                              yyval.logic_formula = logic_create_formula_unary(next,L_EXISTENTIAL);}
break;
case 6:
#line 55 "logics.y"
{Formula *until = logic_create_formula_binary(yyvsp[-2].logic_formula,yyvsp[0].logic_formula,L_UNTIL);
                                              yyval.logic_formula = logic_create_formula_unary(until,L_GLOBAL);}
break;
case 7:
#line 57 "logics.y"
{Formula *until = logic_create_formula_binary(yyvsp[-2].logic_formula,yyvsp[0].logic_formula,L_UNTIL);
                                              yyval.logic_formula = logic_create_formula_unary(until,L_EXISTENTIAL);}
break;
case 8:
#line 60 "logics.y"
{/*A[]formula = E<>!formula*/
                                              /*!formula*/
                                              Formula *not_formula = logic_create_formula_unary(yyvsp[0].logic_formula,L_NEGATION);
                                              Formula *until = logic_create_formula_binary(NULL,not_formula,L_UNTIL);
                                              Formula *existential = logic_create_formula_unary(until,L_EXISTENTIAL);
                                              yyval.logic_formula = logic_create_formula_unary(existential,L_NEGATION);}
break;
case 9:
#line 66 "logics.y"
{Formula *until = logic_create_formula_binary(NULL,yyvsp[0].logic_formula,L_UNTIL);
                                              yyval.logic_formula = logic_create_formula_unary(until,L_GLOBAL);}
break;
case 10:
#line 68 "logics.y"
{/* E[]formula = A<>!formula */
                                              /*!formula*/
                                              Formula *not_formula = logic_create_formula_unary(yyvsp[0].logic_formula,L_NEGATION);
                                              Formula *until = logic_create_formula_binary(NULL,not_formula,L_UNTIL);
                                              Formula *global = logic_create_formula_unary(until,L_GLOBAL);
                                              yyval.logic_formula = logic_create_formula_unary(global,L_NEGATION);}
break;
case 11:
#line 74 "logics.y"
{Formula *until = logic_create_formula_binary(NULL,yyvsp[0].logic_formula,L_UNTIL);
                                              yyval.logic_formula = logic_create_formula_unary(until,L_EXISTENTIAL);}
break;
case 12:
#line 76 "logics.y"
{yyval.logic_formula = logic_create_formula_binary(yyvsp[-2].logic_formula,yyvsp[0].logic_formula,L_LEADSTO);}
break;
case 13:
#line 80 "logics.y"
{/*Get Place number*/
                                        int place = net_place_index(yyvsp[0].stval, parsed_net);
                                        if(place < 0){
                                            unboubd_variable(yyvsp[0].stval);
                                        }
                                        /*TODO:: For Data variables*/
                                        yyval.logic_expression = logic_create_property_expression(place, L_NET);}
break;
case 14:
#line 87 "logics.y"
{yyval.logic_expression = logic_create_property_expression(0, L_DEAD);}
break;
case 15:
#line 88 "logics.y"
{yyval.logic_expression=yyvsp[-1].logic_expression;}
break;
case 16:
#line 90 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(NULL,yyvsp[0].logic_expression, L_NOT);}
break;
case 17:
#line 91 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_AND);}
break;
case 18:
#line 92 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_OR);}
break;
case 19:
#line 93 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_SMALLER);}
break;
case 20:
#line 94 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_SMALLER);}
break;
case 21:
#line 95 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_SMALLER);}
break;
case 22:
#line 96 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_SMALLER);}
break;
case 23:
#line 97 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_SMALLER_EQ);}
break;
case 24:
#line 98 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_SMALLER_EQ);}
break;
case 25:
#line 99 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_SMALLER_EQ);}
break;
case 26:
#line 100 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_SMALLER_EQ);}
break;
case 27:
#line 101 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_GREATER);}
break;
case 28:
#line 102 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_GREATER);}
break;
case 29:
#line 103 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_GREATER);}
break;
case 30:
#line 104 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_GREATER);}
break;
case 31:
#line 105 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_GREATER_EQ);}
break;
case 32:
#line 106 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_GREATER_EQ);}
break;
case 33:
#line 107 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_GREATER_EQ);}
break;
case 34:
#line 108 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_GREATER_EQ);}
break;
case 35:
#line 109 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_EQ);}
break;
case 36:
#line 110 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_EQ);}
break;
case 37:
#line 111 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_EQ);}
break;
case 38:
#line 112 "logics.y"
{yyval.logic_expression = logic_create_bool_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_EQ);}
break;
case 39:
#line 116 "logics.y"
{yyval.logic_expression = logic_create_natural_expression(yyvsp[0].natural);}
break;
case 40:
#line 117 "logics.y"
{yyval.logic_expression=yyvsp[-1].logic_expression;}
break;
case 41:
#line 118 "logics.y"
{yyval.logic_expression = logic_create_math_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_PLUS);}
break;
case 42:
#line 119 "logics.y"
{/*Get Place number*/
                                        int place = net_place_index(yyvsp[-2].stval, parsed_net);
                                        /*TODO:: For Data variables*/
                                        Expression *prop = logic_create_property_expression(place, L_NET);
                                        yyval.logic_expression = logic_create_math_expression(prop,yyvsp[0].logic_expression, L_PLUS);}
break;
case 43:
#line 124 "logics.y"
{yyval.logic_expression = logic_create_math_expression(yyvsp[-2].logic_expression,yyvsp[0].logic_expression, L_PROD);}
break;
case 44:
#line 125 "logics.y"
{/*Get Place number*/
                                        int place = net_place_index(yyvsp[-2].stval, parsed_net);
                                        /*TODO:: For Data variables*/
                                        Expression *prop = logic_create_property_expression(place, L_NET);
                                        yyval.logic_expression = logic_create_math_expression(prop,yyvsp[0].logic_expression, L_PROD);}
break;
#line 760 "logics.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}
