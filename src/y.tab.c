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
#define YYPREFIX "yy"
#line 1 "net.y"
        
  #ifndef debug
  #define debug 0
  #endif
  #include "dstruct.h" 
  extern int  yylineno;
  extern char yytext[];
  void yyerror(char* msg);
  int yylex(void);
  int sym[26];
  int noerror=1;
  AvlType *transitions = NULL;
  AvlType *places = NULL;
  AvlType *labels = NULL;
  extern NetParserType *net;
  char *parser_net_name;
#line 18 "net.y"
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
#line 53 "y.tab.c"
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
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,   10,   10,   10,   10,   10,   10,    3,    3,    6,
    6,   11,   11,   12,   12,   13,   13,   13,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    2,
    2,    2,    8,    8,    8,    4,    9,    9,    9,    9,
   14,   14,    5,    7,
};
short yylen[] = {                                         2,
    1,    0,    2,    2,    2,    2,    2,    7,    4,    0,
    2,    0,    3,    2,    2,    2,    2,    2,    0,    4,
    4,    3,    4,    3,    4,    3,    4,    3,    2,    0,
    4,    2,    2,    2,    1,    5,    0,    4,    4,    3,
    0,    3,    3,    2,
};
short yydefred[] = {                                      2,
    0,    0,    7,    0,    0,    0,    0,    4,    5,    6,
    3,   44,    0,    0,    0,    0,    0,    0,   43,   11,
    0,    0,    0,    0,    0,    0,   15,   14,    0,    0,
    0,    0,    0,   36,    0,    0,    0,    0,    0,   29,
    0,    0,    0,   13,    0,    0,   40,    0,    0,   28,
    0,   24,    0,    0,   22,    0,   26,    0,    0,    8,
   18,   17,   16,   38,   39,   42,   33,   34,   27,   23,
   20,   21,   25,    0,   32,    0,   31,
};
short yydgoto[] = {                                       1,
   30,   60,    8,    9,   10,   17,   11,   51,   26,    2,
   23,   24,   44,   34,
};
short yysindex[] = {                                      0,
    0, -231,    0, -248, -247, -240, -237,    0,    0,    0,
    0,    0,  -44,  -44, -233, -227, -250,    4,    0,    0,
 -226, -219, -216,   -4, -215, -216,    0,    0,  -31, -214,
 -256,  -38, -212,    0, -253, -253, -213, -253, -253,    0,
 -211, -202, -242,    0,   20,   21,    0, -211, -230,    0,
 -216,    0, -216, -216,    0, -216,    0, -216,  -42,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, -213,    0, -211,    0,
};
short yyrindex[] = {                                      0,
    0,   64,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    1,    9,    0,    0,   16,   24,    0,    0,
    0,    0,   39,    0,    0,   46,    0,    0, -200,    0,
    0,    0,    0,    0, -200, -200,    0, -200, -200,    0,
   53,    0,    0,    0,    0,    0,    0,   53,   31,    0,
 -200,    0, -200, -200,    0, -200,    0, -200,   53,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   53,    0,
};
short yygindex[] = {                                      0,
   12,  -40,    0,    0,    0,   57,    0,  -32,    0,    0,
    0,    0,    0,    0,
};
#define YYTABLESIZE 315
short yytable[] = {                                      74,
   10,   39,   47,   53,   54,   56,   58,   66,   10,   42,
   37,   21,   22,   16,   43,   12,   29,   49,   75,   62,
   63,   12,   13,   37,    3,    4,    5,    6,    7,   14,
   35,   38,   15,   67,   68,   77,   19,   33,    9,   31,
   40,   76,   20,   25,   27,   41,   50,   52,   10,   55,
   57,   28,   30,   29,   41,   32,   48,   49,   59,   61,
   64,   65,   69,    1,   70,   71,    0,   72,   19,   73,
   18,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   45,   46,   59,    0,    0,
    0,    0,    0,    0,    0,   35,   36,    0,   29,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   10,   10,   10,   10,
   10,    0,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   12,   12,   12,   12,   12,    0,   10,   10,   37,
   37,   37,   37,   37,   12,   12,   35,   35,   35,   35,
   35,    0,   37,   37,    9,    9,    9,    9,    9,   35,
   35,   41,   41,   41,   41,   41,    0,   19,   30,   30,
   30,   30,   30,    0,   19,
};
short yycheck[] = {                                      42,
    0,   33,   41,   36,   37,   38,   39,   48,    0,  266,
   42,  262,  263,   58,  271,    0,  270,  271,   59,  262,
  263,  270,  270,    0,  256,  257,  258,  259,  260,  270,
    0,   63,  270,  264,  265,   76,  270,   26,    0,   44,
   29,   74,  270,   40,  271,    0,   35,   36,   40,   38,
   39,  271,    0,  270,  269,  271,  269,  271,  270,  262,
   41,   41,   51,    0,   53,   54,   -1,   56,  269,   58,
   14,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  264,  265,  270,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  267,  268,   -1,  270,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,  257,  258,  259,
  260,   -1,  262,  263,  256,  257,  258,  259,  260,  269,
  270,  256,  257,  258,  259,  260,   -1,  269,  270,  256,
  257,  258,  259,  260,  269,  270,  256,  257,  258,  259,
  260,   -1,  269,  270,  256,  257,  258,  259,  260,  269,
  270,  256,  257,  258,  259,  260,   -1,  269,  256,  257,
  258,  259,  260,   -1,  269,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 271
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,0,0,0,"'('","')'","'*'",0,"','",0,0,0,0,0,0,0,0,0,0,0,0,0,"':'",0,0,
0,0,"'?'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"NET","TR","PL","LB","PR","LBRACK","RBRACK","KP","MP","INFI","STOPW","INIB",
"ARROW","IDENT","NAT",
};
char *yyrule[] = {
"$accept : program",
"program : descs",
"descs :",
"descs : descs netdesc",
"descs : descs trdesc",
"descs : descs pldesc",
"descs : descs lbdesc",
"descs : descs error",
"trdesc : TR IDENT label interval arc1 ARROW arc2",
"trdesc : TR IDENT label interval",
"label :",
"label : ':' IDENT",
"interval :",
"interval : left ',' right",
"left : RBRACK NAT",
"left : LBRACK NAT",
"right : NAT RBRACK",
"right : NAT LBRACK",
"right : INFI LBRACK",
"arc1 :",
"arc1 : IDENT '*' weight arc1",
"arc1 : IDENT '?' weight arc1",
"arc1 : IDENT '?' arc1",
"arc1 : IDENT INIB weight arc1",
"arc1 : IDENT INIB arc1",
"arc1 : IDENT '!' weight arc1",
"arc1 : IDENT '!' arc1",
"arc1 : IDENT STOPW weight arc1",
"arc1 : IDENT STOPW arc1",
"arc1 : IDENT arc1",
"arc2 :",
"arc2 : IDENT '*' weight arc2",
"arc2 : IDENT arc2",
"weight : NAT KP",
"weight : NAT MP",
"weight : NAT",
"pldesc : PL IDENT label marking arc_places",
"marking :",
"marking : '(' NAT KP ')'",
"marking : '(' NAT MP ')'",
"marking : '(' NAT ')'",
"arc_places :",
"arc_places : arc1 ARROW arc2",
"lbdesc : LB IDENT IDENT",
"netdesc : NET IDENT",
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
#line 340 "net.y"
#include <stdio.h>
#include "flags.h"

#include "reset_define_includes.h"
#define STDIOLIB
#define STDLIB
#include "standard_includes.h"

extern char linebuf[256];
void yyerror(char* msg)
{
 noerror=0;
  printf("Line %d: %s at \" %s \"\n",yylineno, msg, yytext);

}
#line 315 "y.tab.c"
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
#line 47 "net.y"
{net=net_parser_create(parser_net_name, &transitions, &places);
                             yyval.net=net;}
break;
case 2:
#line 50 "net.y"
{}
break;
case 3:
#line 51 "net.y"
{parser_net_name=yyvsp[0].ptr;}
break;
case 4:
#line 52 "net.y"
{avl_search_insert(&transitions, yyvsp[0].transition, &tr_compare, &tr_free);}
break;
case 5:
#line 53 "net.y"
{avl_search_insert(&places, yyvsp[0].place, &pl_compare, &pl_free);}
break;
case 6:
#line 54 "net.y"
{avl_search_insert(&labels, yyvsp[0].label, &lb_compare, &lb_free);}
break;
case 7:
#line 55 "net.y"
{yyerrok ; yyclearin;}
break;
case 8:
#line 60 "net.y"
{ char *name=(char*)malloc((1 + strlen(yyvsp[-5].stval))*sizeof(char));
               strcpy(name,yyvsp[-5].stval);
               yyval.transition = tr_create(name, yyvsp[-4].ptr, yyvsp[-2].list, yyvsp[0].list);}
break;
case 9:
#line 64 "net.y"
{ char *name=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
               strcpy(name,yyvsp[-2].stval);
               yyval.transition = tr_create(name, yyvsp[-1].ptr, NULL, NULL);}
break;
case 10:
#line 68 "net.y"
{char *name=(char*)malloc((1 + strlen("no label"))*sizeof(char));
                             strcpy(name,"no label");
                             yyval.ptr = name;}
break;
case 11:
#line 71 "net.y"
{char *name=(char*)malloc((1 + strlen(yyvsp[0].stval))*sizeof(char));
                            strcpy(name,yyvsp[0].stval);
                            yyval.ptr=name;}
break;
case 12:
#line 75 "net.y"
{}
break;
case 13:
#line 76 "net.y"
{}
break;
case 14:
#line 79 "net.y"
{}
break;
case 15:
#line 80 "net.y"
{}
break;
case 16:
#line 83 "net.y"
{}
break;
case 17:
#line 84 "net.y"
{}
break;
case 18:
#line 85 "net.y"
{}
break;
case 19:
#line 88 "net.y"
{yyval.list=list_init_empty();}
break;
case 20:
#line 89 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;                                    
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-3].stval);
                                    strcpy(name2,yyvsp[-3].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, NORMAL, yyvsp[-1].weight_value);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                  }
break;
case 21:
#line 106 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-3].stval);
                                    strcpy(name2,yyvsp[-3].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, TEST, yyvsp[-1].weight_value);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                  }
break;
case 22:
#line 123 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-2].stval);
                                    strcpy(name2,yyvsp[-2].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, TEST, NULL);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                  }
break;
case 23:
#line 140 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-3].stval);
                                    strcpy(name2,yyvsp[-3].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, INHIBITOR, yyvsp[-1].weight_value);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                  }
break;
case 24:
#line 157 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-2].stval);
                                    strcpy(name2,yyvsp[-2].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, INHIBITOR, NULL);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                  }
break;
case 25:
#line 174 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-3].stval);
                                    strcpy(name2,yyvsp[-3].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, STOPWATCH, yyvsp[-1].weight_value);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                  }
break;
case 26:
#line 191 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-2].stval);
                                    strcpy(name2,yyvsp[-2].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, STOPWATCH, NULL);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                  }
break;
case 27:
#line 208 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-3].stval);
                                    strcpy(name2,yyvsp[-3].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, STOPWATCH_INHIBITOR, yyvsp[-1].weight_value);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                   }
break;
case 28:
#line 225 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-2].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-2].stval);
                                    strcpy(name2,yyvsp[-2].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, STOPWATCH_INHIBITOR, NULL);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                   }
break;
case 29:
#line 242 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-1].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-1].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-1].stval);
                                    strcpy(name2,yyvsp[-1].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    WeightType *w=NULL;
                                    w=weight_create(1, ZERO);
                                    ArcType *new_arc = arc_create(name, NORMAL, w);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                  }
break;
case 30:
#line 262 "net.y"
{yyval.list=list_init_empty();}
break;
case 31:
#line 263 "net.y"
{ ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-3].stval);
                                    strcpy(name2,yyvsp[-3].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    ArcType *new_arc = arc_create(name, NORMAL, yyvsp[-1].weight_value);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                   }
break;
case 32:
#line 280 "net.y"
{ListType *list;
                                    if (yyvsp[0].list==NULL){
                                        list = list_init_empty();
                                    } else {
                                        list = yyvsp[0].list;
                                    }
                                    PlType *pl=NULL;
                                    char *name=(char*)malloc((1 + strlen(yyvsp[-1].stval))*sizeof(char));
                                    char *name2=(char*)malloc((1 + strlen(yyvsp[-1].stval))*sizeof(char));
                                    strcpy(name,yyvsp[-1].stval);
                                    strcpy(name2,yyvsp[-1].stval);
                                    pl=place_create(name2,NULL,NULL);
                                    avl_search_insert(&places, pl, &pl_compare, &pl_free);
                                    WeightType *w=NULL;
                                    w=weight_create(1, ZERO);
                                    ArcType *new_arc = arc_create(name, NORMAL, w);
                                    list_append(list, new_arc);
                                    yyval.list=list;
                                   }
break;
case 33:
#line 300 "net.y"
{yyval.weight_value=weight_create(yyvsp[-1].natural, K);}
break;
case 34:
#line 301 "net.y"
{yyval.weight_value=weight_create(yyvsp[-1].natural, M);}
break;
case 35:
#line 302 "net.y"
{yyval.weight_value=weight_create(yyvsp[0].natural, ZERO);}
break;
case 36:
#line 304 "net.y"
{char *name=(char*)malloc((1 + strlen(yyvsp[-3].stval))*sizeof(char));
                                             strcpy(name,yyvsp[-3].stval);
                                             yyval.place=place_create(name,yyvsp[-2].ptr,yyvsp[-1].weight_value);}
break;
case 37:
#line 308 "net.y"
{yyval.weight_value=weight_create(0, ZERO);}
break;
case 38:
#line 309 "net.y"
{yyval.weight_value=weight_create(yyvsp[-2].natural, K);}
break;
case 39:
#line 310 "net.y"
{yyval.weight_value=weight_create(yyvsp[-2].natural, M);}
break;
case 40:
#line 311 "net.y"
{yyval.weight_value=weight_create(yyvsp[-1].natural, ZERO);}
break;
case 41:
#line 313 "net.y"
{}
break;
case 42:
#line 314 "net.y"
{}
break;
case 43:
#line 316 "net.y"
{ char *name, *namefor;
                                name = (char*)malloc((1 + strlen(yyvsp[-1].stval))*sizeof(char));
                                strcpy(name,yyvsp[-1].stval);
                                namefor = (char*)malloc((1 + strlen(yyvsp[0].stval))*sizeof(char));
                                strcpy(namefor,yyvsp[0].stval); 
                                yyval.label=label_create(name , namefor);}
break;
case 44:
#line 334 "net.y"
{char *name=(char*)malloc((1 + strlen(yyvsp[0].stval))*sizeof(char));
                            strcpy(name,yyvsp[0].stval);
                            yyval.ptr=name;}
break;
#line 883 "y.tab.c"
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
