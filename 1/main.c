#include"tree.h"
#include"lex.yy.c"
extern void yyrestart(FILE *input_file);
extern int yyparse(void);
extern char *yytext;
// create new leaf node
struct Ast* newLeaf(char* s,int yyline)
{
    struct Ast *l=(struct Ast *)malloc(sizeof(struct Ast));
    l->name=s; //unit syntax's name
    l->line=yyline;
    l->n=0;
    if((!strcmp(l->name,"ID"))||(!strcmp(l->name,"TYPE")))
    {
        char *t;
        t=(char *)malloc(sizeof(char *)*10);
        strcpy(t,yytext);
        l->type=t;
    }
    else if(!strcmp(l->name,"INT"))
        l->i=atoi(yytext);
    else if(!strcmp(l->name,"FLOAT"))
        l->f=atof(yytext);
    return l;
}
//create new syntax tree's node
struct Ast *newNode(char *s,int yyline,int num,struct Ast* arr[])
{
    struct Ast *l=(struct Ast *)malloc(sizeof(struct Ast));
    l->name=s;

    l->line=yyline;
    l->n=num;
    for(int i=0;i<l->n;i++)
        l->child[i]=arr[i];
    return l;
}
//empty process
struct Ast *newNode0(char *s,int num)
{
    struct Ast *l=(struct Ast *)malloc(sizeof(struct Ast));
    l->name=s;
    l->n=0;
    l->line=-1;
    return l;
}
//tree traversal
void printTree(struct Ast* r,int layer)
{
    if(r!=NULL && r->line!=-1)
    {
        for(int i=0;i<layer;i++)
            printf("  ");
        printf("%s",r->name);//unit's name
        if((!strcmp(r->name,"ID"))||(!strcmp(r->name,"TYPE")))
            printf(": %s",r->type);
        else if(!strcmp(r->name,"INT"))
            printf(": %d",r->i);
        else if(!strcmp(r->name,"FLOAT"))
            printf(": %f",r->f);
        else if(r->n!=0)  //unit syntax's output
            printf(" (%d)",r->line);
        printf("\n");
        for(int k=0;k<r->n;k++)
            printTree(r->child[k],layer+1);
    }
}
int main(int argc, char** argv) {
    if(argc<=1)
        return 1;
    FILE* f=fopen(argv[1],"r");
    if(!f)
    {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    return 0;
}
