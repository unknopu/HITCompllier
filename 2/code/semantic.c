#include "semantic.h"
#include "syntax.tab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int functionDepth;
int structureDepth;
int noNameStructureNum;

FieldList HashTable[TableSize];
FieldList FuncDep[FuncDepSize];
FieldList curStructure[FuncDepSize];

unsigned int hash_pjw(char *name) //open hasing method
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~0x3fff)
            val = (val ^ (i >> 12)) & 0x3fff;
    }

    return val;
}

int del_cur_fun_dep()
{
    FieldList f1 = FuncDep[functionDepth];
    FieldList f2;
    while (f1 != NULL)
    {
        f2 = f1;
        f1 = f1->tail;

        unsigned int hash = hash_pjw(f2->name) % TableSize;
        if (HashTable[hash] != NULL && strcmp(HashTable[hash]->name, f2->name) == 0 && HashTable[hash]->depth == functionDepth)
        {
            HashTable[hash] = HashTable[hash]->tail;
        }
    }
}

FieldList insert_hash_table(FieldList fieldList, int kind)
{
    FieldList hashfieldList;
    unsigned int hash = hash_pjw(fieldList->name) % TableSize;

    if (kind == FUNCTION)
    {
        functionDepth--;
    }

    if (HashTable[hash] == NULL)
    {
        HashTable[hash] = (FieldList)malloc(sizeof(struct FieldList_));
        HashTable[hash]->name = fieldList->name;
        HashTable[hash]->type = fieldList->type;
        HashTable[hash]->tail = NULL;
        HashTable[hash]->depth = functionDepth;
    }
    else
    {
        hashfieldList = (FieldList)malloc(sizeof(struct FieldList_));
        hashfieldList->tail = HashTable[hash];
        HashTable[hash] = hashfieldList;
        hashfieldList->name = fieldList->name;
        hashfieldList->type = fieldList->type;
        hashfieldList->depth = functionDepth;
    }

    FieldList funDepth = (FieldList)malloc(sizeof(struct FieldList_));
    funDepth->tail = FuncDep[functionDepth];
    funDepth->depth = functionDepth;
    funDepth->name = HashTable[hash]->name;
    funDepth->type = HashTable[hash]->type;
    FuncDep[functionDepth] = funDepth;

    if (kind == FUNCTION)
    {
        functionDepth++;
        HashTable[hash]->type->kind = FUNCTION;
    }
    return NULL;
}

FieldList search_hash_table(char *name, Type type, int SpecialCode)
{
    unsigned int hash = hash_pjw(name) % TableSize;
    FieldList fieldList = HashTable[hash];

    if (SpecialCode == e_ExtDef)
    {
        while (fieldList != NULL)
        {
            if (strcmp(name, fieldList->name) == 0 && fieldList->type->kind == FUNCTION)
            {
                return fieldList;
            }
            fieldList = fieldList->tail;
        }
        return NULL;
    }
    else if (SpecialCode == e_OptTag)
    {
        while (fieldList != NULL)
        {
            if (strcmp(name, fieldList->name) == 0)
            {
                if (fieldList->type->kind == STRUCTURE || fieldList->type->kind == BASIC || fieldList->type->kind == ARRAY)
                {
                    return fieldList;
                }
            }
            fieldList = fieldList->tail;
        }
        return NULL;
    }
    else if (SpecialCode == e_Tag)
    {
        while (fieldList != NULL)
        {
            if (strcmp(name, fieldList->name) == 0 && fieldList->type->kind == STRUCTURE)
            {
                return fieldList;
            }
            fieldList = fieldList->tail;
        }
        return NULL;
    }
    else if (SpecialCode == e_VarDec)
    {
        while (fieldList != NULL)
        {
            if (strcmp(name, fieldList->name) == 0 && fieldList->type->kind != FUNCTION && functionDepth <= fieldList->depth)
            {
                return fieldList;
            }

            if (strcmp(name, fieldList->name) == 0 && fieldList->type->kind != FUNCTION && fieldList->type->value == LEFT_VALUE)
            {
                return fieldList;
            }

            fieldList = fieldList->tail;
        }
        return NULL;
    }
    else if (SpecialCode == e_Exp)
    {
        while (fieldList != NULL)
        {
            if (strcmp(name, fieldList->name) == 0 && fieldList->type->kind != FUNCTION)
            {
                return fieldList;
            }
            fieldList = fieldList->tail;
        }
        return NULL;
    }
    else if (SpecialCode == e_NOT)
    {
        while (fieldList != NULL)
        {
            if (strcmp(name, fieldList->name) == 0)
            {
                return fieldList;
            }
            fieldList = fieldList->tail;
        }
        return NULL;
    }
    else
    {
        while (fieldList != NULL)
        {
            if (strcmp(name, fieldList->name) == 0)
            {
                return fieldList;
            }
            fieldList = fieldList->tail;
        }
    }
    return NULL;
}

int type_check(Type t1, Type t2)
{
    if (t1 == NULL && t2 != NULL || t2 == NULL && t1 != NULL || t1 == NULL && t2 == NULL)
    {
        return -1;
    }

    if (t1->kind != t2->kind)
    {
        return -1;
    }

    if (t1->kind == BASIC)
    {
        if (t1->u.basic == t2->u.basic)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    if (t1->kind == ARRAY)
    {
        return type_check(t1->u.array.elem, t2->u.array.elem);
    }

    if (t1->kind == STRUCTURE)
    {
        if (t1->u.structure->name == NULL && t2->u.structure->name == NULL && t1->curNonNameStruct == t2->curNonNameStruct)
        {
            return 0;
        }

        if (t1->u.structure->name == NULL || t2->u.structure->name == NULL)
        {
            return -1;
        }

        if (strcmp(t1->u.structure->name, t2->u.structure->name) == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    if (t1->kind == FUNCTION)
    {
        if (type_check(t1->u.function.returnType, t2->u.function.returnType) == 0 && t1->u.function.paramNum == t2->u.function.paramNum)
        {
            FieldList funF1 = t1->u.function.params;
            FieldList funF2 = t2->u.function.params;
            while (funF1 != NULL && funF2 != NULL)
            {
                if (type_check(funF1->type, funF2->type) != 0)
                {
                    return -1;
                }
                funF1 = funF1->tail;
                funF2 = funF2->tail;
            }
            return 0;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}

void create_hash_table()
{
    int i;
    for (i = 0; i < TableSize; i++)
    {
        HashTable[i] = NULL;
    }

    functionDepth = 0;
    for (i = 0; i < FuncDepSize; i++)
    {
        FuncDep[i] = NULL;
    }

    for (i = 0; i < FuncDepSize; i++)
    {
        curStructure[i] = NULL;
    }

    structureDepth = 1;
    curStructure[0] = (FieldList)malloc(sizeof(struct FieldList_));
    curStructure[0]->tail = NULL;
    curStructure[0]->name = NULL;
    curStructure[0]->type = NULL;

    noNameStructureNum = 0;
}

void semantic_analysis(struct TreeNode *root)
{
    create_hash_table();
    Program(root);
}

void Program(struct TreeNode *root)
{
    ExtDefList(root->child);
}

void ExtDefList(struct TreeNode *extdeflist)
{
    if (extdeflist->int_name == e_NULL || extdeflist == NULL || extdeflist->child == NULL)
    {
        return;
    }

    ExtDef(extdeflist->child);
    ExtDefList(extdeflist->child->sibling);
}

void ExtDef(struct TreeNode *extdef)
{
    if (extdef == NULL)
    {
        return;
    }

    Type returnType = Specifier(extdef->child);
    FieldList f1 = NULL;
    FieldList f2;

    switch (extdef->child->sibling->int_name)
    {
    case e_ExtDecList:
        ExtDecList(extdef->child->sibling, returnType);
        break;

    case e_SEMI:
        return;

    case e_FunDec:
        functionDepth++;
        f1 = FunDec(extdef->child->sibling, returnType);

        if (f1 != NULL)
        {
            f2 = search_hash_table(f1->name, f1->type, e_ExtDef);
            if (f2 != NULL)
            {
                printf("Error type 4 at Line %d: Redefined function \"%s\".\n",
                       extdef->child->sibling->child->lineno,
                       extdef->child->sibling->child->nchar_data);
            }
            else
            {
                insert_hash_table(f1, FUNCTION);
            }
            CompSt(extdef->child->sibling->sibling, returnType);
            del_cur_fun_dep();
            functionDepth--;
        }
        break;

    default:
        break;
    }
}

void ExtDecList(struct TreeNode *extdeclist, Type type)
{
    if (extdeclist->child->sibling == NULL)
    {
        VarDec(extdeclist->child, type, BASIC);
    }
    else
    {
        VarDec(extdeclist->child, type, BASIC);
        ExtDecList(extdeclist->child->sibling->sibling, type);
    }
}

Type Specifier(struct TreeNode *specifier)
{
    if (specifier == NULL)
    {
        return NULL;
    }

    Type type;
    switch (specifier->child->int_name)
    {
    case e_TYPE:
        type = (Type)malloc(sizeof(struct Type_));
        type->kind = BASIC;

        if (strcmp(specifier->child->nchar_data, "int") == 0)
        {
            type->u.basic = UBASIC_INT;
        }
        else
        {
            type->u.basic = UBASIC_FLOAT;
        }
        type->value = RIGHT_VALUE;
        break;

    case e_StructSpecifier:
        type = StructSpecifier(specifier->child);
        break;

    default:
        break;
    }
    return type;
}

Type StructSpecifier(struct TreeNode *structspecifier)
{
    if (structspecifier == NULL)
    {
        return NULL;
    }

    Type t1 = (Type)malloc(sizeof(struct Type_));
    Type t2 = (Type)malloc(sizeof(struct Type_));
    FieldList fieldList;

    switch (structspecifier->child->sibling->int_name)
    {
    case e_NULL:
        t1->curNonNameStruct = noNameStructureNum;
        noNameStructureNum++;

    case e_OptTag:
        t1->kind = STRUCTURE;
        t1->value = LEFT_VALUE;
        fieldList = (FieldList)malloc(sizeof(struct FieldList_));
        t1->u.structure = fieldList;
        fieldList->tail = NULL;

        if (structspecifier->child->sibling->int_name == e_NULL || structspecifier->child->sibling->child->int_name == e_NULL)
        {
            fieldList->name = NULL;
        }
        else
        {
            fieldList->name = structspecifier->child->sibling->child->nchar_data;
        }

        t1->structureName = fieldList->name;

        curStructure[structureDepth] = (FieldList)malloc(sizeof(struct FieldList_));
        curStructure[structureDepth]->tail = NULL;

        structureDepth++;
        fieldList->tail = DefList(structspecifier->child->sibling->sibling->sibling, STRUCTURE);

        fieldList->type = t1;
        if (fieldList->name != NULL)
        {
            FieldList f2 = search_hash_table(fieldList->name, NULL, e_OptTag);
            if (f2 != NULL)
            {
                printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",
                       structspecifier->child->sibling->child->lineno,
                       structspecifier->child->sibling->child->nchar_data);
                return fieldList->type;
            }
            else
            {
                insert_hash_table(fieldList, STRUCTURE);
            }
        }

        structureDepth--;
        memcpy(t2, t1, sizeof(struct Type_));
        return t2;

    case e_Tag:
        fieldList = search_hash_table(structspecifier->child->sibling->child->nchar_data, NULL, e_Tag);

        if (fieldList == NULL)
        {
            printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",
                   structspecifier->child->sibling->child->lineno,
                   structspecifier->child->sibling->child->nchar_data);
            return NULL;
        }

        return fieldList->type;

    default:
        break;
    }

    memcpy(t2, t1, sizeof(struct Type_));
    return t2;
}

FieldList DefList(struct TreeNode *deflist, int structure)
{
    if (deflist->int_name == e_NULL || deflist == NULL || deflist->child == NULL)
    {
        return NULL;
    }

    FieldList fieldList = Def(deflist->child, structure);
    if (fieldList == NULL)
    {
        fieldList = DefList(deflist->child->sibling, structure);
    }
    else
    {
        FieldList tmp = fieldList;
        while (tmp->tail != NULL)
        {
            tmp = tmp->tail;
        }
        tmp->tail = DefList(deflist->child->sibling, structure);
    }
    return fieldList;
}

FieldList Def(struct TreeNode *def, int structure)
{
    if (def == NULL)
    {
        return NULL;
    }

    FieldList fieldList;
    Type type = Specifier(def->child);

    if (type == NULL)
    {
        return NULL;
    }

    fieldList = DecList(def->child->sibling, type, structure);
    return fieldList;
}

FieldList DecList(struct TreeNode *declist, Type type, int structure)
{
    if (declist == NULL || declist->child == NULL)
    {
        return NULL;
    }

    FieldList fieldList = Dec(declist->child, type, structure);

    if (declist->child->sibling != NULL)
    {
        if (fieldList == NULL)
        {
            fieldList = DecList(declist->child->sibling->sibling, type, structure);
        }
        else
        {
            FieldList tmp = fieldList;
            while (tmp->tail != NULL)
            {
                tmp = tmp->tail;
            }
            tmp->tail = DecList(declist->child->sibling->sibling, type, structure);
        }
    }
    return fieldList;
}

FieldList Dec(struct TreeNode *dec, Type type, int structure)
{
    if (dec == NULL)
    {
        return NULL;
    }

    FieldList fieldList = VarDec(dec->child, type, structure);
    if (dec->child->sibling != NULL)
    {
        if (structure == STRUCTURE)
        {
            printf("Error type 15 at Line %d: Cannot initialize field when to define the structure.\n", dec->child->lineno);
        }

        Type t = Exp(dec->child->sibling->sibling);
        if (type_check(t, fieldList->type) != 0)
        {
            printf("Error type 5 at Line %d: Type mismatched for assignment.\n", dec->child->sibling->sibling->lineno);
        }
    }
    return fieldList;
}

FieldList VarDec(struct TreeNode *vardec, Type type2, int structure)
{
    if (vardec == NULL)
    {
        return NULL;
    }

    FieldList f;
    FieldList f2;
    FieldList fieldList1;
    FieldList fieldList2;
    Type type = (Type)malloc(sizeof(struct Type_));
    memcpy(type, type2, sizeof(struct Type_));
    type->value = BOTH_VALUE;
    Type t;

    switch (vardec->child->int_name)
    {
    case e_ID:
        f = (FieldList)malloc(sizeof(struct FieldList_));
        f->name = vardec->child->nchar_data;
        f->type = type;
        f->tail = NULL;

        switch (structure)
        {
        case BASIC:
            f2 = search_hash_table(f->name, f->type, e_VarDec);
            if (f2 != NULL)
            {
                if (curStructure[structureDepth] == NULL)
                {
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",
                           vardec->child->lineno, vardec->child->nchar_data);
                    break;
                }

                FieldList curFl = curStructure[structureDepth]->tail;
                int flag = 0;

                while (curFl != NULL)
                {
                    if (strcmp(f->name, curFl->name) == 0)
                    {
                        flag = 1;
                    }
                    curFl = curFl->tail;
                }

                if (flag == 0)
                {
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",
                           vardec->child->lineno, vardec->child->nchar_data);
                }
                else
                {
                    insert_hash_table(f, BASIC);
                }
            }
            else
            {
                insert_hash_table(f, BASIC);
            }
            break;

        case FUNCTION:
            f2 = search_hash_table(f->name, f->type, e_VarDec);

            if (f2 != NULL)
            {
                if (f2->type->value == LEFT_VALUE && f->type->value == LEFT_VALUE)
                {
                    if (functionDepth <= f->depth)
                    {
                        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",
                               vardec->child->lineno, vardec->child->nchar_data);
                    }
                }
                else
                {
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",
                           vardec->child->lineno, vardec->child->nchar_data);
                }
            }
            else
            {
                insert_hash_table(f, BASIC);
            }
            break;

        case STRUCTURE:
            fieldList1 = (FieldList)malloc(sizeof(struct FieldList_));
            fieldList2 = curStructure[structureDepth - 1];
            while (fieldList2->tail != NULL)
            {
                fieldList2 = fieldList2->tail;
            }

            fieldList2->tail = fieldList1;
            fieldList1->tail = NULL;
            fieldList1->name = f->name;
            fieldList1->type = f->type;

            f2 = search_hash_table(f->name, f->type, e_VarDec);

            if (f2 != NULL)
            {
                FieldList curFl = curStructure[structureDepth - 1]->tail;

                while (curFl->tail != NULL)
                {
                    if (strcmp(f->name, curFl->name) == 0)
                    {
                        printf("Error type 15 at Line %d: Redefined field \"%s\".\n", vardec->child->lineno, vardec->child->nchar_data);
                        break;
                    }
                    curFl = curFl->tail;
                }
            }
            else
            {
                insert_hash_table(f, STRUCTURE);
            }
            break;

        default:
            break;
        }

        return f;
        break;

    case e_VarDec:
        t = (Type)malloc(sizeof(struct Type_));
        t->kind = ARRAY;
        t->u.array.elem = type;
        t->u.array.size = atoi(vardec->child->sibling->sibling->nchar_data);
        return VarDec(vardec->child, t, structure);
        break;

    default:
        break;
    }
    return f;
}

FieldList FunDec(struct TreeNode *fundec, Type type)
{
    if (fundec == NULL)
    {
        return NULL;
    }

    FieldList fieldList = (FieldList)malloc(sizeof(struct FieldList_));
    fieldList->name = fundec->child->nchar_data;

    FieldList f2 = search_hash_table(fieldList->name, NULL, e_ExtDef);

    if (f2 != NULL)
    {
        printf("Error type 4 at Line %d: Redefined function \"%s\".\n", fundec->child->lineno, fundec->child->nchar_data);
    }

    fieldList->type = (Type)malloc(sizeof(struct Type_));
    fieldList->type->kind = FUNCTION;
    fieldList->type->u.function.returnType = type;
    fieldList->type->u.function.params = NULL;

    fieldList->tail = NULL;
    if (fundec->child->sibling->sibling->int_name == e_VarList)
    {
        fieldList->type->u.function.params = VarList(fundec->child->sibling->sibling);
    }
    return fieldList;
}

FieldList VarList(struct TreeNode *varlist)
{
    if (varlist == NULL)
    {
        return NULL;
    }

    FieldList fieldList = ParamDec(varlist->child);

    if (varlist->child->sibling != NULL)
    {
        if (fieldList == NULL)
        {
            fieldList = VarList(varlist->child->sibling->sibling);
        }
        else
        {
            fieldList->tail = VarList(varlist->child->sibling->sibling);
        }
    }
    return fieldList;
}

FieldList ParamDec(struct TreeNode *paramdec)
{
    if (paramdec == NULL)
    {
        return NULL;
    }

    Type type = Specifier(paramdec->child);

    if (type == NULL)
    {
        return NULL;
    }

    return VarDec(paramdec->child->sibling, type, FUNCTION);
}

void CompSt(struct TreeNode *compst, Type type)
{
    if (compst == NULL)
    {
        return;
    }

    DefList(compst->child->sibling, BASIC);
    StmtList(compst->child->sibling->sibling, type);
}

void StmtList(struct TreeNode *stmtlist, Type type)
{
    if (stmtlist->int_name == e_NULL || stmtlist == NULL || stmtlist->child == NULL)
    {
        return;
    }
    Stmt(stmtlist->child, type);
    StmtList(stmtlist->child->sibling, type);
}

void Stmt(struct TreeNode *stmt, Type type)
{
    if (stmt == NULL)
    {
        return;
    }
    Type t;

    switch (stmt->child->int_name)
    {

    case e_Exp:
        Exp(stmt->child);
        break;

    case e_CompSt:
        functionDepth++;
        CompSt(stmt->child, type);
        del_cur_fun_dep();
        functionDepth--;
        break;

    case e_RETURN:
        t = Exp(stmt->child->sibling);
        if (type_check(t, type) != 0)
        {
            printf("Error type 8 at Line %d: Type mismatched for return.\n", stmt->child->lineno);
        }
        break;

    case e_IF:
        Exp(stmt->child->sibling->sibling);
        Stmt(stmt->child->sibling->sibling->sibling->sibling, type);
        if (stmt->child->sibling->sibling->sibling->sibling->sibling != NULL)
        {
            Stmt(stmt->child->sibling->sibling->sibling->sibling->sibling->sibling, type);
        }
        break;

    case e_WHILE:
        Exp(stmt->child->sibling->sibling);
        Stmt(stmt->child->sibling->sibling->sibling->sibling, type);
        break;

    default:
        break;
    }
}

Type Exp(struct TreeNode *exp)
{
    FieldList fieldList;
    Type t;
    Type t1;
    Type t2;

    switch (exp->child->int_name)
    {
    case e_Exp:
    {
        switch (exp->child->sibling->int_name)
        {
        case e_ASSIGNOP:
            t1 = Exp(exp->child);
            t2 = Exp(exp->child->sibling->sibling);

            if (t1 == NULL || t2 == NULL)
            {
                return NULL;
            }
            if (t1->value == RIGHT_VALUE)
            {
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", exp->child->lineno);
                return NULL;
            }
            if (type_check(t1, t2) != 0)
            {
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n", exp->child->lineno);
                return NULL;
            }
            return t1;

        case e_AND:
        case e_OR:
        case e_RELOP:
        case e_PLUS:
        case e_MINUS:
        case e_STAR:
        case e_DIV:
            t1 = Exp(exp->child);
            t2 = Exp(exp->child->sibling->sibling);

            if (t1 == NULL || t2 == NULL)
            {
                return NULL;
            }
            if (t1->kind == BASIC && t2->kind == BASIC && t1->u.basic == t2->u.basic)
            {
                t1->value = RIGHT_VALUE;
                return t1;
            }
            else
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n", exp->child->lineno);
                return NULL;
            }
            break;

        case e_LB:
            t = Exp(exp->child);

            if (t == NULL)
            {
                return NULL;
            }

            if (t->kind != ARRAY)
            {
                printf("Error type 10 at Line %d: \"%s\" is not an array.\n", exp->child->lineno, exp->child->child->nchar_data);
                return NULL;
            }

            Type t2 = Exp(exp->child->sibling->sibling);
            if (t2 == NULL)
            {
                return NULL;
            }

            if (t2->kind == BASIC && t2->u.basic == UBASIC_INT)
            {
                Type typeDbg = (Type)malloc(sizeof(struct Type_));
                memcpy(typeDbg, t->u.array.elem, sizeof(struct Type_));
                typeDbg->value = BOTH_VALUE;
                return typeDbg;
            }
            else
            {
                printf("Error type 12 at Line %d: \"%s\" is not an integer.\n",
                       exp->child->lineno,
                       exp->child->sibling->sibling->child->nchar_data);
                return NULL;
            }
            break;

        case e_DOT:
            t = Exp(exp->child);

            if (t == NULL)
            {
                return NULL;
            }

            if (t->kind != STRUCTURE)
            {
                printf("Error type 13 at Line %d: Illegal use of \".\".\n", exp->child->lineno);
                return NULL;
            }

            fieldList = t->u.structure->tail;

            while (fieldList != NULL)
            {
                if (strcmp(fieldList->name, exp->child->sibling->sibling->nchar_data) == 0)
                {
                    Type t2 = (Type)malloc(sizeof(struct Type_));
                    memcpy(t2, fieldList->type, sizeof(struct Type_));
                    t2->value = BOTH_VALUE;
                    return t2;
                }
                fieldList = fieldList->tail;
            }
            printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",
                   exp->child->lineno, exp->child->sibling->sibling->nchar_data);
            return NULL;

        default:
            break;
        }
    }
    break;

    case e_LP:
        return Exp(exp->child->sibling);

    case e_MINUS:
    case e_NOT:
        t = Exp(exp->child->sibling);
        if (t == NULL)
        {
            return NULL;
        }

        if (t->kind != BASIC)
        {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", exp->child->lineno);
            return NULL;
        }

        t->value = RIGHT_VALUE;
        return t;

    case e_ID:
    {
        if (exp->child->sibling == NULL)
        {
            fieldList = search_hash_table(exp->child->nchar_data, NULL, e_Exp);

            if (fieldList == NULL || fieldList->type->kind == STRUCTURE && fieldList->type->value == LEFT_VALUE)
            {
                printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", exp->child->lineno, exp->child->nchar_data);
                return NULL;
            }
            else
            {
                t = (Type)malloc(sizeof(struct Type_));
                memcpy(t, fieldList->type, sizeof(struct Type_));

                t->value = BOTH_VALUE;
                return t;
            }
        }
        else if (exp->child->sibling->sibling->sibling == NULL)
        {
            fieldList = search_hash_table(exp->child->nchar_data, NULL, e_NOT);
            if (fieldList == NULL)
            {
                printf("Error type 2 at Line %d: Undefined function \"%s\".\n", exp->child->lineno, exp->child->nchar_data);
                return NULL;
            }
            else if (fieldList->type->kind != FUNCTION)
            {
                printf("Error type 11 at Line %d: \"%s\" is not a function.\n", exp->child->lineno, fieldList->name);
                return NULL;
            }
            else if (fieldList->type->u.function.params != NULL)
            {
                printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",
                       exp->child->lineno, exp->child->nchar_data);
                return NULL;
            }
            else
            {
                t = (Type)malloc(sizeof(struct Type_));
                memcpy(t, fieldList->type->u.function.returnType, sizeof(struct Type_));

                t->value = RIGHT_VALUE;
                return t;
            }
        }
        else
        {
            fieldList = search_hash_table(exp->child->nchar_data, NULL, e_NOT);
            if (fieldList == NULL)
            {
                printf("Error type 2 at Line %d: Undefined function \"%s\".\n", exp->child->lineno, exp->child->nchar_data);
                return NULL;
            }
            else if (fieldList->type->kind != FUNCTION)
            {
                printf("Error type 11 at Line %d: \"%s\" is not a function.\n", exp->child->lineno, fieldList->name);
                return NULL;
            }
            else
            {
                FieldList tmpfl1 = fieldList->type->u.function.params;
                FieldList tmpfl2 = Args(exp->child->sibling->sibling);

                int same = 1;
                while (tmpfl1 != NULL && tmpfl2 != NULL)
                {
                    if (type_check(tmpfl1->type, tmpfl2->type) == 0)
                    {
                        tmpfl1 = tmpfl1->tail;
                        tmpfl2 = tmpfl2->tail;
                    }
                    else
                    {
                        same = 0;
                        break;
                    }
                }
                if (tmpfl1 == NULL && tmpfl2 == NULL && same == 1)
                {
                    t = (Type)malloc(sizeof(struct Type_));
                    memcpy(t, fieldList->type->u.function.returnType, sizeof(struct Type_));
                    t->value = RIGHT_VALUE;
                    return t;
                }
                else
                {
                    printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",
                           exp->child->lineno, exp->child->nchar_data);
                    return NULL;
                }
            }
        }
    }
    break;

    case e_INT:
        t = (Type)malloc(sizeof(struct Type_));
        t->kind = BASIC;
        t->u.basic = UBASIC_INT;
        t->value = RIGHT_VALUE;
        return t;

    case e_FLOAT:
        t = (Type)malloc(sizeof(struct Type_));
        t->kind = BASIC;
        t->u.basic = UBASIC_FLOAT;
        t->value = RIGHT_VALUE;
        return t;

    default:
        break;
    }
}

FieldList Args(struct TreeNode *args)
{
    FieldList fieldList = (FieldList)malloc(sizeof(struct FieldList_));
    fieldList->type = Exp(args->child);
    fieldList->name = NULL;
    fieldList->tail = NULL;

    if (args->child->sibling != NULL)
    {
        if (fieldList == NULL)
        {
            fieldList = Args(args->child->sibling->sibling);
        }
        else
        {
            FieldList tmp = fieldList;
            while (tmp->tail != NULL)
            {
                tmp = tmp->tail;
            }
            tmp->tail = Args(args->child->sibling->sibling);
        }
    }
    return fieldList;
}