#include <stdio.h>
#include "syntax.tab.h"
#include "semantic.h"
#include "inter.h"

extern FILE *yyin;
extern int yylineno;
extern int haserror;
extern void yyrestart(FILE *);
extern struct TreeNode *root;
extern void printTree(struct TreeNode *tree, int depth);

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        return 1;
    }

    FILE *f = fopen(argv[1], "r");

    if (!f)
    {
        perror(argv[1]);
        return 1;
    }

    yylineno = 1;
    yyrestart(f);
    yyparse();

    if (!haserror)
    {
        semantic_analysis(root);
        fprintInterCode(argv[2]);
    }
    return 0;
}