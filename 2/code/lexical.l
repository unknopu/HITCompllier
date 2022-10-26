%{
#include "syntax.tab.h"
#include <string.h>

extern int haserror;

enum treetype
{
    type_char, type_int, type_float, type_nchar
};

enum treename
{
    e_SEMI, e_COMMA, e_TYPE,
    e_LC, e_RC, e_LB, e_RB, e_LP, e_RP,
    e_STRUCT, e_ID, e_INT, e_FLOAT,
    e_ASSIGNOP, e_AND, e_OR, e_RELOP,
    e_PLUS, e_MINUS, e_STAR, e_DIV,
    e_NOT, e_DOT,
    e_IF, e_ELSE, e_WHILE, e_RETURN,
    e_Program, e_ExtDefList, e_ExtDef, e_ExtDecList, e_Specifier,
    e_FunDec, e_CompSt, e_VarDec, e_VarList, e_StructSpecifier, e_OptTag, e_DefList,
    e_Args, e_ParamDec, e_Exp, e_Stmt, e_StmtList, e_Tag, e_Def, e_DecList, e_Dec,
    e_NULL
};

struct TreeNode
{
    int lineno;
    union
    {
        int type;
        char *char_type;
    };

    union
    {
        int name;
        char *char_name;
    };

    union
    {
        int int_data;
        float float_data;
        char *char_data;
        char nchar_data[40];
    };

    struct TreeNode *child;
    struct TreeNode *sibling;
};

int newtoken(int type, int name, char *data);

int yycolumn = 1;

#define YY_USER_ACTION                               \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn;                  \
    yylloc.last_column = yycolumn + yyleng - 1;      \
    yycolumn += yyleng;

%}

%option yylineno

newline \n
delim [ \t\n\r]
ws {delim}+
digit [0-9]
digits {digit}+
letter [_a-zA-Z]
INT [1-9]{digit}*|0
FLOAT {INT}\.{digit}+|{digit}+\.{digit}*[eE][-+]?{digit}+|\.{digit}+[eE][-+]?{digit}+
ID {letter}({letter}|{digit})*
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT \!
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while

comment1    (\/\/[^\n]*)
comment2    (\/\*((([^\*])|(\*[^\/]))*)\*\/)
comment     {comment1}|{comment2}

%%

{comment} {}

{newline} {yycolumn=1;}
{ws} {/* no action and no return */}

{TYPE} {newtoken(type_char, e_TYPE, yytext); return TYPE;}
{STRUCT} {newtoken(type_char, e_STRUCT, yytext); return STRUCT;}

{IF} {newtoken(type_char, e_IF, yytext); return IF;}
{ELSE} {newtoken(type_char, e_ELSE, yytext); return ELSE;}
{WHILE} {newtoken(type_char, e_WHILE, yytext); return WHILE;}
{RETURN} {newtoken(type_char, e_RETURN, yytext); return RETURN;}

{SEMI} {newtoken(type_char, e_SEMI, yytext); return SEMI;}
{COMMA} {newtoken(type_char, e_COMMA, yytext); return COMMA;}
{ASSIGNOP} {newtoken(type_char, e_ASSIGNOP, yytext); return ASSIGNOP;}

{LP} {newtoken(type_char, e_LP, yytext); return LP;}
{RP} {newtoken(type_char, e_RP, yytext); return RP;}
{LB} {newtoken(type_char, e_LB, yytext); return LB;}
{RB} {newtoken(type_char, e_RB, yytext); return RB;}
{LC} {newtoken(type_char, e_LC, yytext); return LC;}
{RC} {newtoken(type_char, e_RC, yytext); return RC;}

{AND} {newtoken(type_char, e_AND, yytext); return AND;}
{OR} {newtoken(type_char, e_OR, yytext); return OR;}

{PLUS} {newtoken(type_char, e_PLUS, yytext); return PLUS;}
{MINUS} {newtoken(type_char, e_MINUS, yytext); return MINUS;}
{STAR} {newtoken(type_char, e_STAR, yytext); return STAR;}
{DIV} {newtoken(type_char, e_DIV, yytext); return DIV;}

{NOT} {newtoken(type_char, e_NOT, yytext); return NOT;}
{DOT} {newtoken(type_char, e_DOT, yytext); return DOT;}

{RELOP} {newtoken(type_char, e_RELOP, yytext); return RELOP;}  
{INT} {newtoken(type_int, e_INT, yytext);return INT;}
{FLOAT} {newtoken(type_float, e_FLOAT, yytext);return FLOAT;}
{ID} {newtoken(type_char, e_ID, yytext); return ID;}

. {haserror=1; printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);}

%%

int newtoken(int type, int name, char *data)
{
    yylval.treeNode = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    yylval.treeNode->lineno = yylineno;
    yylval.treeNode->type = type;
    yylval.treeNode->name = name;

    switch (type)
    {
    case type_char:
        strcpy(yylval.treeNode->nchar_data, data);
        break;

    case type_int:
        yylval.treeNode->int_data = atoi(data);
        break;

    case type_float:
        yylval.treeNode->float_data = atof(data);
        break;
    }

    yylval.treeNode->child = NULL;
    yylval.treeNode->sibling = NULL;
    return 0;
}