#ifndef SEMANTIC
#define SEMANTIC

typedef struct Type_ *Type;
typedef struct FieldList_ *FieldList;

#define TableSize 0x3fff
#define FuncDepSize 0x3fff

struct TreeNode
{
    int lineno;
    union
    {
        int int_type;
        char *char_type;
    };
    union
    {
        int int_name;
        char *char_name;
    };
    union
    {
        int int_data;
        float float_data;
        char nchar_data[40];
    };
    struct TreeNode *child;
    struct TreeNode *sibling;
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

    e_Program, e_ExtDefList, e_ExtDef, e_ExtDecList,
    e_Specifier, e_FunDec, e_CompSt, e_VarDec, e_VarList, e_StructSpecifier, e_OptTag,
    e_DefList, e_Args, e_ParamDec, e_Exp, e_Stmt, e_StmtList, e_Tag, e_Def, e_DecList, e_Dec,

    e_NULL
};

struct Type_
{
    enum
    {
        BASIC,
        ARRAY,
        STRUCTURE,
        FUNCTION
    } kind;

    union
    {
        enum
        {
            UBASIC_INT,
            UBASIC_FLOAT
        } basic;
        struct
        {
            Type elem;
            int size;
        } array;

        FieldList structure;

        struct
        {
            Type returnType;
            int paramNum;
            FieldList params;
        } function;
    } u;

    char *structureName;
    int curNonNameStruct;

    enum
    {
        LEFT_VALUE,
        RIGHT_VALUE,
        BOTH_VALUE,
        OTHERS_VALUE
    } value;
};

struct FieldList_
{
    char *name;
    Type type;
    FieldList tail;
    int depth;
};

unsigned int hash_pjw(char *name);

FieldList insert_hash_table(FieldList fieldList, int kind);
FieldList search_hash_table(char *name, Type typ, int specialCode);

void create_hash_table();
void semantic_analysis(struct TreeNode *root);
void Program(struct TreeNode *root);
void ExtDefList(struct TreeNode *extdeflist);
void ExtDecList(struct TreeNode *extdeclist, Type type);
void ExtDef(struct TreeNode *extdef);
Type Specifier(struct TreeNode *specifier);
Type StructSpecifier(struct TreeNode *structspecifier);
FieldList DefList(struct TreeNode *deflist, int structure);
FieldList Def(struct TreeNode *def, int structure);
FieldList DecList(struct TreeNode *declist, Type type, int structure);
FieldList Dec(struct TreeNode *dec, Type type, int structure);
FieldList VarDec(struct TreeNode *vardec, Type type, int structure);
FieldList FunDec(struct TreeNode *fundec, Type type);
FieldList VarList(struct TreeNode *fundec);
FieldList ParamDec(struct TreeNode *paramdec);
void CompSt(struct TreeNode *compst, Type type);
void StmtList(struct TreeNode *stmtlist, Type type);
void Stmt(struct TreeNode *stmt, Type type);
Type Exp(struct TreeNode *exp);
FieldList Args(struct TreeNode *args);

int type_check(Type t1, Type t2);

#endif