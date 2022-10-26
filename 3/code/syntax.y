%{
#include "lex.yy.c"
#include <stdio.h>
#include <string.h>

struct TreeNode *root;
void add(struct TreeNode *t);
void printTree(struct TreeNode *tree, int layer);
extern void yyerror(char *msg);

int haserror = 0;

extern enum treetype treetype;
extern enum treename treename;

char *tokenArray[] = {
    "SEMI", "COMMA", "TYPE", "LC", "RC", "LB", "RB", "LP", "RP", "STRUCT", "ID", "INT", "FLOAT", "ASSIGNOP",
    "AND", "OR", "RELOP", "PLUS", "MINUS", "STAR", "DIV", "NOT", "DOT", "IF", "ELSE", "WHILE", "RETURN",
    "Program", "ExtDefList", "ExtDef", "ExtDecList", "Specifier",
    "FunDec", "CompSt", "VarDec", "VarList", "StructSpecifier", "OptTag", "DefList",
    "Args", "ParamDec", "Exp", "Stmt", "StmtList", "Tag", "Def", "DecList", "Dec",
    "NULL"};

struct TreeNode *newnode(enum treename tn, enum treetype tt);

%}

%union{
    struct TreeNode* treeNode;
}

%locations

/* vatiables */
%token <treeNode> INT FLOAT ID

/* sentence end symbol */
%token <treeNode> SEMI COMMA ASSIGNOP RELOP DOT NOT

/* operational symbol */
%token <treeNode> PLUS MINUS STAR DIV

/* brackets */
%token <treeNode> LC RC LB RB LP RP

/* reserved keywords */
%token <treeNode> TYPE STRUCT IF ELSE WHILE RETURN AND OR

/* non terminal */
%type <treeNode> Program ExtDefList ExtDef ExtDecList Specifier 
%type <treeNode> FunDec CompSt VarDec VarList StructSpecifier OptTag DefList
%type <treeNode> Args ParamDec Exp Stmt StmtList Tag Def DecList Dec

/* first or last */
%nonassoc errorSEMI
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS 
%left STAR DIV
%right NOT
%left LB RB LP RP DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/* definitions */

/* high level */
Program : ExtDefList {$$ = newnode(e_Program, type_nchar); $$->child = $1; add($$); root= $$;}
    ;

ExtDefList : ExtDef ExtDefList {$$ = newnode(e_ExtDefList, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}
    | /* empty */ {$$ = newnode(e_NULL, type_nchar); $$->child =NULL; add($$);}
    ;

ExtDef : Specifier ExtDecList SEMI {$$ = newnode(e_ExtDef, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Specifier SEMI {$$ = newnode(e_ExtDef, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}
    | error SEMI {}
    | Specifier FunDec CompSt {$$ = newnode(e_ExtDef, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    ;

ExtDecList : VarDec {$$ = newnode(e_ExtDecList, type_nchar); $$->child = $1; add($$);}
    | VarDec COMMA ExtDecList {$$ = newnode(e_ExtDecList, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    ;

/* specifiers */
Specifier : TYPE {$$ = newnode(e_Specifier, type_nchar); $$->child = $1; add($$);}
    | StructSpecifier {$$ = newnode(e_Specifier, type_nchar); $$->child = $1; add($$);}
    ;

StructSpecifier : STRUCT OptTag LC DefList RC {$$ = newnode(e_StructSpecifier, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; $4->sibling = $5; add($$);}
    | STRUCT Tag {$$ = newnode(e_StructSpecifier, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}    

    ;
OptTag : ID {$$ = newnode(e_OptTag, type_nchar); $$->child = $1; add($$);}
    | /*empty*/ {$$ = newnode(e_NULL, type_nchar); $$->child =NULL; add($$);}
    ;
Tag :ID {$$ = newnode(e_Tag, type_nchar); $$->child = $1; add($$);}
    ;

/* declarators */
VarDec : ID {$$ = newnode(e_VarDec, type_nchar); $$->child = $1; add($$);}
    | VarDec LB INT RB {$$ = newnode(e_VarDec, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; add($$);}
    | VarDec LB error RB {}
    ;
FunDec : ID LP VarList RP {$$ = newnode(e_FunDec, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; add($$);}
    | ID LP RP {$$ = newnode(e_FunDec, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | error RP {}
    ;

VarList : ParamDec COMMA VarList {$$ = newnode(e_VarList, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | ParamDec {$$ = newnode(e_VarList, type_nchar); $$->child = $1; add($$);}
    ;

ParamDec : Specifier VarDec {$$ = newnode(e_ParamDec, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}
    ;

/* statements */
CompSt : LC DefList StmtList RC {$$ = newnode(e_CompSt, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; add($$);}
    | error RC {}
    ;

StmtList : Stmt StmtList {$$ = newnode(e_StmtList, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}
    | /* empty */ {$$ = newnode(e_NULL, type_nchar); $$->child =NULL; add($$);}
    ;
    
Stmt : Exp SEMI {$$ = newnode(e_Stmt, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}
    | CompSt {$$ = newnode(e_Stmt, type_nchar); $$->child = $1; add($$);}
    | RETURN Exp SEMI {$$ = newnode(e_Stmt, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE Stmt {$$ = newnode(e_Stmt, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; $4->sibling = $5; add($$);}
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE Stmt {}
    | IF LP Exp RP Stmt ELSE Stmt {$$ = newnode(e_Stmt, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; $4->sibling = $5; $5->sibling = $6; $6->sibling = $7; add($$);}
    | IF LP error RP Stmt ELSE Stmt {}
    | WHILE LP Exp RP Stmt {$$ = newnode(e_Stmt, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; $4->sibling = $5; add($$);}
    | WHILE LP error RP Stmt {}
    | error SEMI {}    
    ;

/* local definitions */
DefList : Def DefList {$$ = newnode(e_DefList, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}
    | /* empty */ {$$ = newnode(e_NULL, type_nchar); $$->child =NULL; add($$);}
    ;

Def : Specifier DecList SEMI {$$ = newnode(e_Def, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Specifier error SEMI {}
    ;

DecList : Dec {$$ = newnode(e_DecList, type_nchar); $$->child = $1; add($$);}
    | Dec COMMA DecList {$$ = newnode(e_DecList, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    ;

Dec : VarDec {$$ = newnode(e_Dec, type_nchar); $$->child = $1; add($$);}
    | VarDec ASSIGNOP Exp {$$ = newnode(e_Dec, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    ;

/* expressions */
Exp : Exp ASSIGNOP Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Exp AND Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Exp OR Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Exp RELOP Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Exp PLUS Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Exp MINUS Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Exp STAR Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Exp DIV Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | LP Exp RP {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | MINUS Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}
    | NOT Exp {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; add($$);}
    | ID LP Args RP {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3;; $3->sibling = $4; add($$);}
    | ID LP RP {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | Exp LB Exp RB {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3;; $3->sibling = $4; add($$);}
    | Exp DOT ID {$$ = newnode(e_Exp, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
    | ID {$$ = newnode(e_Exp, type_nchar); $$->child = $1; add($$);}
    | INT {$$ = newnode(e_Exp, type_nchar); $$->child = $1; add($$);}
    | FLOAT {$$ = newnode(e_Exp, type_nchar); $$->child = $1; add($$);}
    | error ASSIGNOP Exp {}
    | error Exp %prec errorSEMI{}
    | Exp LB error RB {}
    ;

Args : Exp COMMA Args {$$ = newnode(e_Args, type_nchar); $$->child = $1; $1->sibling = $2; $2->sibling = $3; add($$);}
     | Exp {$$ = newnode(e_Args, type_nchar); $$->child = $1; add($$);}
     ;

%%

struct TreeNode *newnode(enum treename tn, enum treetype tt)
{
    struct TreeNode *t = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    t->type = tt;
    t->name = tn;
    switch (tt)
    {
    case type_char:
    case type_int:
    case type_float:
    case type_nchar:
        strcpy(t->nchar_data, tokenArray[tn]);
        break;
    }
    t->child = NULL;
    t->sibling = NULL;
    return t;
}

void add(struct TreeNode *t)
{
    if (t != NULL && t->child != NULL)
    {
        t->lineno = t->child->lineno;
    }
}

void yyerror(char *msg)
{
    haserror = 1;
    fprintf(stderr, "Error type B at line %d: %s.\n", yylineno, msg);
}

void printTree(struct TreeNode *tree, int layer)
{

    if (tree == NULL)
    {
        return;
    }

    if (tree->name == e_Program && tree->child->child == NULL)
    {
        printf("%s (%d)\n", tokenArray[tree->name], yylineno);
        return;
    }
    
    if (tree->name != e_NULL)
    {
        int i = 0;
        for (; i < layer; i++)
        {
            printf("  ");
        }

        switch (tree->type)
        {
        case type_char:
            if (tree->name == e_ID || tree->name == e_TYPE)
            {
                printf("%s: %s", tokenArray[tree->name], tree->nchar_data);
            }
            else
            {
                printf("%s", tokenArray[tree->name]);
            }
            break;

        case type_int:
            printf("%s: %d", tokenArray[tree->name], tree->int_data);
            break;

        case type_float:
            printf("%s: %f", tokenArray[tree->name], tree->float_data);
            break;

        case type_nchar:
            printf("%s (%d)", tokenArray[tree->name], tree->lineno);
            break;

        default:
            break;
        }

        printf("\n");
    }

    printTree(tree->child, layer + 1);
    printTree(tree->sibling, layer);
}
