#include "semantic.h"
#include "syntax.tab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int structureDepth;
int noNameStructNum;

FieldList curStructure[TableSize];
FieldList HashTable[TableSize];

unsigned int hash_pjw(char *name)
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

FieldList insert_hash_table(FieldList fieldList, int kind)
{
    unsigned int hash = hash_pjw(fieldList->name) % TableSize;
    FieldList fl;

    if (HashTable[hash] == NULL)
    {
        HashTable[hash] = (FieldList)malloc(sizeof(struct FieldList_));
        HashTable[hash]->name = fieldList->name;
        HashTable[hash]->type = fieldList->type;
        HashTable[hash]->tail = NULL;
    }
    else
    {
        fl = (FieldList)malloc(sizeof(struct FieldList_));
        fl->tail = HashTable[hash];
        HashTable[hash] = fl;
        fl->name = fieldList->name;
        fl->type = fieldList->type;
    }

    if (kind == FUNCTION)
    {
        HashTable[hash]->type->kind = FUNCTION;
    }

    return NULL;
}

FieldList search_hash_table(char *name, Type type, int specialCode)
{
    unsigned int hash = hash_pjw(name) % TableSize;
    FieldList fieldList = HashTable[hash];

    if (specialCode == e_ExtDef)
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
    else if (specialCode == e_OptTag)
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
    else if (specialCode == e_Tag)
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
    else if (specialCode == e_VarDec)
    {
        while (fieldList != NULL)
        {
            if (strcmp(name, fieldList->name) == 0 && fieldList->type->kind != FUNCTION)
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
    else if (specialCode == e_Exp)
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
    else if (specialCode == e_NOT)
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
            if (strcmp(name, fieldList->name) == 0 && fieldList->type->kind != FUNCTION)
            {
                return fieldList;
            }
            fieldList = fieldList->tail;
        }
        return NULL;
    }
}

void create_hash_table()
{
    int i;
    for (i = 0; i < TableSize; i++)
    {
        HashTable[i] = NULL;
    }

    FieldList fun1 = (FieldList)malloc(sizeof(struct FieldList_));
    memset(fun1, 0, sizeof(struct FieldList_));
    fun1->name = (char *)malloc(sizeof(char) * 40);
    strcpy(fun1->name, "read");
    fun1->type = (Type)malloc(sizeof(struct Type_));
    memset(fun1->type, 0, sizeof(struct Type_));

    fun1->tail = NULL;
    fun1->type->kind = FUNCTION;
    fun1->type->u.function.returnType = (Type)malloc(sizeof(struct Type_));

    memset(fun1->type->u.function.returnType, 0, sizeof(struct Type_));

    fun1->type->u.function.returnType->kind = BASIC;
    fun1->type->u.function.returnType->u.basic = UBASIC_INT;
    fun1->type->u.function.params = NULL;
    fun1->type->u.function.paramNum = 0;

    insert_hash_table(fun1, READWRITE);

    FieldList fun2 = (FieldList)malloc(sizeof(struct FieldList_));
    memset(fun2, 0, sizeof(struct FieldList_));
    fun2->name = (char *)malloc(sizeof(char) * 40);
    strcpy(fun2->name, "write");
    fun2->type = (Type)malloc(sizeof(struct Type_));
    memset(fun2->type, 0, sizeof(struct Type_));

    fun2->tail = NULL;
    fun2->type->kind = FUNCTION;
    fun2->type->u.function.returnType = (Type)malloc(sizeof(struct Type_));
    memset(fun2->type->u.function.returnType, 0, sizeof(struct Type_));

    fun2->type->u.function.returnType->kind = BASIC;
    fun2->type->u.function.returnType->u.basic = UBASIC_INT;
    fun2->type->u.function.params = (FieldList)malloc(sizeof(struct FieldList_));

    memset(fun2->type->u.function.params, 0, sizeof(struct FieldList_));

    char *write = (char *)malloc(40 * sizeof(char));
    strcpy(write, "wparams");

    fun2->type->u.function.params->name = write;
    fun2->type->u.function.params->type = (Type)malloc(sizeof(struct Type_));

    memset(fun2->type->u.function.params->type, 0, sizeof(struct Type_));

    fun2->type->u.function.params->type->kind = BASIC;
    fun2->type->u.function.paramNum = 1;

    insert_hash_table(fun2, READWRITE);

    for (i = 0; i < 0x3fff; i++)
    {
        curStructure[i] = NULL;
    }

    structureDepth = 1;
    curStructure[0] = (FieldList)malloc(sizeof(struct FieldList_));
    curStructure[0]->tail = NULL;
    curStructure[0]->name = NULL;
    curStructure[0]->type = NULL;

    noNameStructNum = 0;
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
    FieldList fieldList = NULL;

    switch (extdef->child->sibling->int_name)
    {
    case e_ExtDecList:
        ExtDecList(extdef->child->sibling, returnType);
        break;

    case e_SEMI:
        return;

    case e_FunDec:
    {
        fieldList = FunDec(extdef->child->sibling, returnType);

        if (fieldList != NULL)
        {
            if (search_hash_table(fieldList->name, fieldList->type, e_ExtDef) != NULL)
            {
            }
            else
            {
                insert_hash_table(fieldList, FUNCTION);

                Operand op = (Operand)malloc(sizeof(struct Operand_));
                op->kind = OP_FUNCTION;
                strcpy(op->u.value, fieldList->name);
                interCode ic = (interCode)malloc(sizeof(struct interCode_));
                ic->kind = IC_FUNCTION;
                ic->u.sinop.op1 = op;
                insertInterCode(ic);
                FieldList fl = fieldList->type->u.function.params;

                while (fl != NULL)
                {
                    Operand opr = (Operand)malloc(sizeof(struct Operand_));
                    opr->kind = OP_VARIABLE;
                    strcpy(opr->u.value, fl->name);
                    interCode ic = (interCode)malloc(sizeof(struct interCode_));
                    ic->kind = IC_PARAM;
                    ic->u.sinop.op1 = opr;
                    insertInterCode(ic);
                    fl = fl->tail;
                }
            }
            CompSt(extdef->child->sibling->sibling, returnType);
        }
        break;
    }
    default:
        break;
    }
}

void ExtDecList(struct TreeNode *extdeclist, Type type)
{
    FieldList l3 = VarDec(extdeclist->child, type, BASIC);
    if (l3 != NULL)
    {
        if (l3->type->kind == ARRAY)
        {
            Operand op = (Operand)malloc(sizeof(struct Operand_));
            op->kind = OP_TMP_VARIABLE;
            op->u.var_num = temp_var_num;
            temp_var_num++;

            interCode ic = (interCode)malloc(sizeof(struct interCode_));
            ic->kind = IC_DEC;
            ic->u.decop.op1 = op;
            ic->u.decop.size = array_length(l3->type, 1);
            insertInterCode(ic);

            Operand opr = (Operand)malloc(sizeof(struct Operand_));
            opr->kind = OP_VARIABLE;
            strcpy(opr->u.value, l3->name);

            interCode icaddr = (interCode)malloc(sizeof(struct interCode_));
            icaddr->kind = IC_GET_ADDR1;
            icaddr->u.binop.left = opr;
            icaddr->u.binop.right = op;
            insertInterCode(icaddr);
        }
    }

    if (extdeclist->child->sibling == NULL)
    {
    }
    else
    {
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
        printf("Cannot translate: Code contains variables or parameters of structure type.\n");
        exit(0);
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
        t1->noNameStruct = noNameStructNum;
        noNameStructNum++;

    case e_OptTag:
        fieldList = (FieldList)malloc(sizeof(struct FieldList_));
        t1->kind = STRUCTURE;
        t1->value = LEFT_VALUE;
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

        fieldList->tail = DefList(structspecifier->child->sibling->sibling->sibling, STRUCTURE);
        fieldList->type = t1;
        structureDepth++;

        if (fieldList->name != NULL)
        {
            if (search_hash_table(fieldList->name, NULL, e_OptTag) != NULL)
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
        FieldList temp = fieldList;
        while (temp->tail != NULL)
        {
            temp = temp->tail;
        }
        temp->tail = DefList(deflist->child->sibling, structure);
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
            FieldList temp = fieldList;
            while (temp->tail != NULL)
            {
                temp = temp->tail;
            }
            temp->tail = DecList(declist->child->sibling->sibling, type, structure);
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

    if (fieldList->type->kind == ARRAY && structure == BASIC)
    {
        Operand op1 = (Operand)malloc(sizeof(struct Operand_));
        op1->kind = OP_TMP_VARIABLE;
        op1->u.var_num = temp_var_num;
        temp_var_num++;

        interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
        ic1->kind = IC_DEC;
        ic1->u.decop.op1 = op1;
        ic1->u.decop.size = array_length(fieldList->type, 1);
        insertInterCode(ic1);

        Operand op2 = (Operand)malloc(sizeof(struct Operand_));
        op2->kind = OP_VARIABLE;
        strcpy(op2->u.value, fieldList->name);

        interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
        ic2->kind = IC_GET_ADDR1;
        ic2->u.binop.left = op2;
        ic2->u.binop.right = op1;
        insertInterCode(ic2);
    }

    if (dec->child->sibling != NULL)
    {
        if (structure == STRUCTURE)
        {
            printf("Error type 15 at Line %d: Cannot initialize field when to define the structure.\n", dec->child->lineno);
        }

        Operand opr = (Operand)malloc(sizeof(struct Operand_));
        opr->kind = OP_VARIABLE;
        strcpy(opr->u.value, fieldList->name);
        Type t = Exp(dec->child->sibling->sibling, opr);

        if (type_check(t, fieldList->type) != 0)
        {
            printf("Error type 5 at Line %d: Type mismatched for assignment.\n", dec->child->sibling->sibling->lineno);
            return NULL;
        }

        if (opr->kind != OP_VARIABLE || strcmp(opr->u.value, fieldList->name) != 0)
        {
            Operand op = (Operand)malloc(sizeof(struct Operand_));
            op->kind = OP_VARIABLE;
            strcpy(op->u.value, fieldList->name);

            interCode ica = (interCode)malloc(sizeof(struct interCode_));
            ica->kind = IC_ASSIGN;
            ica->u.binop.left = op;
            ica->u.binop.right = opr;

            search_hash_table(op->u.value, NULL, -1);
            search_hash_table(opr->u.value, NULL, -1);
            insertInterCode(ica);
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

    FieldList fieldList;
    FieldList fieldList1;
    FieldList fieldList2;

    Type type = (Type)malloc(sizeof(struct Type_));
    memcpy(type, type2, sizeof(struct Type_));
    type->value = BOTH_VALUE;
    Type t;

    switch (vardec->child->int_name)
    {
    case e_ID:
    {
        fieldList = (FieldList)malloc(sizeof(struct FieldList_));
        fieldList->name = vardec->child->nchar_data;
        fieldList->type = type;
        fieldList->tail = NULL;

        switch (structure)
        {
        case BASIC:
            if (search_hash_table(fieldList->name, fieldList->type, e_VarDec) != NULL)
            {
                if (curStructure[structureDepth] == NULL)
                {
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", vardec->child->lineno, vardec->child->nchar_data);
                    break;
                }

                FieldList fl1 = curStructure[structureDepth]->tail;
                int flag = 0;
                while (fl1 != NULL)
                {
                    if (strcmp(fieldList->name, fl1->name) == 0)
                    {
                        flag = 1;
                    }
                    fl1 = fl1->tail;
                }

                if (flag == 0)
                {
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", vardec->child->lineno, vardec->child->nchar_data);
                }
                else
                {
                    insert_hash_table(fieldList, BASIC);
                }
            }
            else
            {
                insert_hash_table(fieldList, BASIC);
            }
            break;

        case FUNCTION:
            if (search_hash_table(fieldList->name, fieldList->type, e_VarDec) != NULL)
            {
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", vardec->child->lineno, vardec->child->nchar_data);
            }
            else
            {
                insert_hash_table(fieldList, BASIC);
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
            fieldList1->name = fieldList->name;
            fieldList1->type = fieldList->type;

            if (search_hash_table(fieldList->name, fieldList->type, e_VarDec) != NULL)
            {
                FieldList fl1 = curStructure[structureDepth - 1]->tail;
                while (fl1->tail != NULL)
                {
                    if (strcmp(fieldList->name, fl1->name) == 0)
                    {
                        printf("Error type 15 at Line %d: Redefined field \"%s\".\n", vardec->child->lineno, vardec->child->nchar_data);
                        break;
                    }
                    fl1 = fl1->tail;
                }
            }
            else
            {
                insert_hash_table(fieldList, STRUCTURE);
            }
            break;

        default:
            break;
        }

        return fieldList;
    }

    case e_VarDec:
        t = (Type)malloc(sizeof(struct Type_));
        t->kind = ARRAY;
        t->u.array.elem = type;
        t->u.array.size = atoi(vardec->child->sibling->sibling->nchar_data);
        return VarDec(vardec->child, t, structure);

    default:
        break;
    }
    return fieldList;
}

FieldList FunDec(struct TreeNode *fundec, Type type)
{
    if (fundec == NULL)
    {
        return NULL;
    }

    FieldList fieldList = (FieldList)malloc(sizeof(struct FieldList_));
    fieldList->name = fundec->child->nchar_data;

    if (search_hash_table(fieldList->name, NULL, e_ExtDef) != NULL)
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
        Exp(stmt->child, NULL);
        break;

    case e_CompSt:
        CompSt(stmt->child, type);
        break;

    case e_RETURN:
    {
        Operand op1 = (Operand)malloc(sizeof(struct Operand_));
        op1->kind = OP_TMP_VARIABLE;
        op1->u.var_num = temp_var_num;
        temp_var_num++;

        t = Exp(stmt->child->sibling, op1);
        if (type_check(t, type) != 0)
        {
            printf("Error type 8 at Line %d: Type mismatched for return.\n", stmt->child->lineno);
        }

        interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
        ic1->kind = IC_RETURN;
        ic1->u.sinop.op1 = op1;
        insertInterCode(ic1);
        break;
    }
    case e_IF:
    {
        Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
        opr1->kind = OP_LABEL;
        opr1->u.var_num = lbl_num;
        lbl_num++;
        Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
        opr2->kind = OP_LABEL;
        opr2->u.var_num = lbl_num;
        lbl_num++;

        condition_translate(stmt->child->sibling->sibling, opr1, opr2);

        interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
        ic1->kind = IC_LABEL;
        ic1->u.sinop.op1 = opr1;
        insertInterCode(ic1);

        Stmt(stmt->child->sibling->sibling->sibling->sibling, type);

        if (stmt->child->sibling->sibling->sibling->sibling->sibling == NULL)
        {
            interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
            ic2->kind = IC_LABEL;
            ic2->u.sinop.op1 = opr2;
            insertInterCode(ic2);
        }
        else
        {
            Operand opr3 = (Operand)malloc(sizeof(struct Operand_));
            opr3->kind = OP_LABEL;
            opr3->u.var_num = lbl_num;
            lbl_num++;

            interCode icg = (interCode)malloc(sizeof(struct interCode_));
            icg->kind = IC_GOTO;
            icg->u.sinop.op1 = opr3;
            insertInterCode(icg);

            interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
            ic2->kind = IC_LABEL;
            ic2->u.sinop.op1 = opr2;
            insertInterCode(ic2);

            Stmt(stmt->child->sibling->sibling->sibling->sibling->sibling->sibling, type);

            interCode ic3 = (interCode)malloc(sizeof(struct interCode_));
            ic3->kind = IC_LABEL;
            ic3->u.sinop.op1 = opr3;
            insertInterCode(ic3);
        }
        break;
    }
    case e_WHILE:
    {
        Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
        opr1->kind = OP_LABEL;
        opr1->u.var_num = lbl_num;
        lbl_num++;

        Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
        opr2->kind = OP_LABEL;
        opr2->u.var_num = lbl_num;
        lbl_num++;

        Operand opr3 = (Operand)malloc(sizeof(struct Operand_));
        opr3->kind = OP_LABEL;
        opr3->u.var_num = lbl_num;
        lbl_num++;

        interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
        ic1->kind = IC_LABEL;
        ic1->u.sinop.op1 = opr1;

        insertInterCode(ic1);
        condition_translate(stmt->child->sibling->sibling, opr2, opr3);

        interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
        ic2->kind = IC_LABEL;
        ic2->u.sinop.op1 = opr2;

        insertInterCode(ic2);
        Stmt(stmt->child->sibling->sibling->sibling->sibling, type);

        interCode icg = (interCode)malloc(sizeof(struct interCode_));
        icg->kind = IC_GOTO;
        icg->u.sinop.op1 = opr1;
        insertInterCode(icg);

        interCode ic3 = (interCode)malloc(sizeof(struct interCode_));
        ic3->kind = IC_LABEL;
        ic3->u.sinop.op1 = opr3;
        insertInterCode(ic3);
        break;
    }
    default:
        break;
    }
}

Type Exp(struct TreeNode *exp, Operand place)
{
    FieldList fieldList;
    Type type;
    Type t1;
    Type t2;
    switch (exp->child->int_name)
    {
    case e_Exp:
    {
        switch (exp->child->sibling->int_name)
        {
        case e_ASSIGNOP:
        {
            Operand opt1 = (Operand)malloc(sizeof(struct Operand_));
            opt1->kind = OP_TMP_VARIABLE;
            opt1->u.var_num = temp_var_num;
            temp_var_num++;
            Operand opt2 = (Operand)malloc(sizeof(struct Operand_));
            opt2->kind = OP_TMP_VARIABLE;
            opt2->u.var_num = temp_var_num;
            temp_var_num++;
            int ron = opt2->u.var_num;

            t1 = Exp(exp->child, opt1);

            t2 = Exp(exp->child->sibling->sibling, opt2);
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

            if (!(t1->kind == ARRAY && t2->kind == ARRAY))
            {
                interCode ica1 = (interCode)malloc(sizeof(struct interCode_));
                ica1->kind = IC_ASSIGN;
                ica1->u.binop.left = opt1;
                ica1->u.binop.right = opt2;
                insertInterCode(ica1);
                if (place != NULL)
                {
                    interCode ica2 = (interCode)malloc(sizeof(struct interCode_));
                    ica2->kind = IC_ASSIGN;
                    ica2->u.binop.left = place;
                    ica2->u.binop.right = opt1;
                    insertInterCode(ica2);
                }
            }
            else
            {
                int copyNum1 = array_length(t1, 1) / 4;
                int copyNum2 = array_length(t2, 1) / 4;
                if (copyNum1 < copyNum2)
                {
                    copyNum2 = copyNum1;
                }

                for (int i = 0; i < copyNum2; i++)
                {
                    Operand optmp1 = (Operand)malloc(sizeof(struct Operand_));
                    optmp1->kind = OP_TMP_VARIABLE;
                    optmp1->u.var_num = temp_var_num;
                    temp_var_num++;

                    Operand optmp2 = (Operand)malloc(sizeof(struct Operand_));
                    optmp2->kind = OP_TMP_VARIABLE;
                    optmp2->u.var_num = temp_var_num;
                    temp_var_num++;

                    Operand optmp3 = (Operand)malloc(sizeof(struct Operand_));
                    optmp3->kind = OP_TMP_VARIABLE;
                    optmp3->u.var_num = temp_var_num;
                    temp_var_num++;

                    Operand opconst = (Operand)malloc(sizeof(struct Operand_));
                    opconst->kind = OP_CONSTANT;
                    sprintf(opconst->u.value, "%d", i * 4);

                    interCode icadd1 = (interCode)malloc(sizeof(struct interCode_));
                    icadd1->kind = IC_ADD;
                    icadd1->u.triop.op1 = opt1;
                    icadd1->u.triop.op2 = opconst;
                    icadd1->u.triop.result = optmp1;
                    insertInterCode(icadd1);

                    interCode icadd2 = (interCode)malloc(sizeof(struct interCode_));
                    icadd2->kind = IC_ADD;
                    icadd2->u.triop.op1 = opt2;
                    icadd2->u.triop.op2 = opconst;
                    icadd2->u.triop.result = optmp3;
                    insertInterCode(icadd2);

                    interCode icvar1 = (interCode)malloc(sizeof(struct interCode_));
                    icvar1->kind = IC_GET_VAR;
                    icvar1->u.binop.left = optmp2;
                    icvar1->u.binop.right = optmp3;
                    insertInterCode(icvar1);

                    interCode icvar2 = (interCode)malloc(sizeof(struct interCode_));
                    icvar2->kind = IC_TO_VAR;
                    icvar2->u.binop.left = optmp1;
                    icvar2->u.binop.right = optmp2;
                    insertInterCode(icvar2);
                }

                if (place != NULL)
                {
                    interCode ica2 = (interCode)malloc(sizeof(struct interCode_));
                    ica2->kind = IC_ASSIGN;
                    ica2->u.binop.left = place;
                    ica2->u.binop.right = opt1;
                    insertInterCode(ica2);
                }
            }
            return t1;
        }
        case e_PLUS:
        case e_MINUS:
        case e_STAR:
        case e_DIV:
        {
            Operand opt1 = (Operand)malloc(sizeof(struct Operand_));
            opt1->kind = OP_TMP_VARIABLE;
            opt1->u.var_num = temp_var_num;
            temp_var_num++;

            Operand opt2 = (Operand)malloc(sizeof(struct Operand_));
            opt2->kind = OP_TMP_VARIABLE;
            opt2->u.var_num = temp_var_num;
            temp_var_num++;

            int ron = opt2->u.var_num;
            t1 = Exp(exp->child, opt1);
            t2 = Exp(exp->child->sibling->sibling, opt2);

            if (t1 == NULL || t2 == NULL)
            {
                return NULL;
            }
            if (t1->kind == BASIC && t2->kind == BASIC && t1->u.basic == t2->u.basic)
            {
                t1->value = RIGHT_VALUE;
                if (place != NULL)
                {
                    if (place->kind == OP_TMP_VARIABLE && opt1->kind == OP_CONSTANT && opt2->kind == OP_CONSTANT)
                    {
                        int res = 0;
                        switch (exp->child->sibling->int_name)
                        {
                        case e_PLUS:
                            res = atoi(opt1->u.value) + atoi(opt2->u.value);
                            break;

                        case e_MINUS:
                            res = atoi(opt1->u.value) - atoi(opt2->u.value);
                            break;

                        case e_STAR:
                            res = atoi(opt1->u.value) * atoi(opt2->u.value);
                            break;

                        case e_DIV:
                            if (atoi(opt2->u.value) == 0)
                            {
                            }
                            else
                            {
                                res = atoi(opt1->u.value) / atoi(opt2->u.value);
                            }
                            break;

                        default:
                            break;
                        }
                        place->kind = OP_CONSTANT;
                        sprintf(place->u.value, "%d", res);
                    }
                    else
                    {
                        interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
                        ic1->next = NULL;
                        ic1->prev = NULL;

                        switch (exp->child->sibling->int_name)
                        {
                        case e_PLUS:
                            ic1->kind = IC_ADD;
                            break;

                        case e_MINUS:
                            ic1->kind = IC_SUB;
                            break;

                        case e_STAR:
                            ic1->kind = IC_MUL;
                            break;

                        case e_DIV:
                            ic1->kind = IC_DIV;
                            break;

                        default:
                            break;
                        }
                        ic1->u.triop.op1 = opt1;
                        ic1->u.triop.op2 = opt2;
                        ic1->u.triop.result = place;
                        insertInterCode(ic1);
                    }
                }
                return t1;
            }
            else
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n", exp->child->lineno);
                return NULL;
            }
            break;
        }
        case e_AND:
        case e_OR:
        case e_RELOP:
        {
            Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
            opr1->kind = OP_LABEL;
            opr1->u.var_num = lbl_num;
            lbl_num++;

            Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
            opr2->kind = OP_LABEL;
            opr2->u.var_num = lbl_num;
            lbl_num++;

            if (place != NULL)
            {
                interCode ica = (interCode)malloc(sizeof(struct interCode_));
                ica->kind = IC_ASSIGN;
                ica->u.binop.left = place;

                Operand opr0 = (Operand)malloc(sizeof(struct Operand_));
                opr0->kind = OP_CONSTANT;
                sprintf(opr0->u.value, "%d", 0);
                ica->u.binop.right = opr0;
                insertInterCode(ica);
            }

            Type type1 = condition_translate(exp, opr1, opr2);
            interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
            ic1->kind = IC_LABEL;
            ic1->u.sinop.op1 = opr1;
            insertInterCode(ic1);

            if (place != NULL)
            {
                interCode ica = (interCode)malloc(sizeof(struct interCode_));
                ica->kind = IC_ASSIGN;
                ica->u.binop.left = place;

                Operand opr0 = (Operand)malloc(sizeof(struct Operand_));
                opr0->kind = OP_CONSTANT;
                sprintf(opr0->u.value, "1");
                ica->u.binop.right = opr0;
                insertInterCode(ica);
            }

            interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
            ic2->kind = IC_LABEL;
            ic2->u.sinop.op1 = opr2;
            insertInterCode(ic2);
            return type1;
        }

        case e_LB:
        {
            Operand base = (Operand)malloc(sizeof(struct Operand_));
            base->kind = OP_TMP_VARIABLE;
            base->u.var_num = temp_var_num;
            temp_var_num++;
            type = Exp(exp->child, base);

            if (type == NULL)
            {
                return NULL;
            }
            if (type->kind != ARRAY)
            {
                printf("Error type 10 at Line %d: \"%s\" is not an array.\n", exp->child->lineno, exp->child->child->nchar_data);
                return NULL;
            }

            Operand sub = (Operand)malloc(sizeof(struct Operand_));
            sub->kind = OP_TMP_VARIABLE;
            sub->u.var_num = temp_var_num;
            temp_var_num++;

            Type t2 = Exp(exp->child->sibling->sibling, sub);

            if (t2 == NULL)
            {
                return NULL;
            }
            if (t2->kind == BASIC && t2->u.basic == UBASIC_INT)
            {
                Operand width = (Operand)malloc(sizeof(struct Operand_));
                width->kind = OP_TMP_VARIABLE;
                width->u.var_num = temp_var_num;
                temp_var_num++;

                Operand opconst = (Operand)malloc(sizeof(struct Operand_));
                opconst->kind = OP_CONSTANT;

                sprintf(opconst->u.value, "%d", array_length(type, 2));

                interCode icmul = (interCode)malloc(sizeof(struct interCode_));
                icmul->kind = IC_MUL;
                icmul->u.triop.op1 = sub;
                icmul->u.triop.op2 = opconst;
                icmul->u.triop.result = width;

                if (sub->kind == OP_CONSTANT && opconst->kind == OP_CONSTANT)
                {
                    int widthoutput = atoi(sub->u.value) * atoi(opconst->u.value);
                    width->kind = OP_CONSTANT;
                    sprintf(width->u.value, "%d", widthoutput);
                }
                else if (sub->kind == OP_CONSTANT && atoi(sub->u.value) == 0 || opconst->kind == OP_CONSTANT && atoi(opconst->u.value) == 0)
                {
                    width->kind = OP_CONSTANT;
                    sprintf(width->u.value, "0");
                }
                else
                {
                    insertInterCode(icmul);
                }

                if (place != NULL)
                {
                    interCode icaddr2 = (interCode)malloc(sizeof(struct interCode_));
                    icaddr2->kind = IC_GET_ADDR2;
                    icaddr2->u.triop.op1 = base;
                    icaddr2->u.triop.op2 = width;
                    icaddr2->u.triop.result = place;
                    if (type->u.array.elem->kind == BASIC)
                    {
                        Operand tmp = (Operand)malloc(sizeof(struct Operand_));
                        tmp->kind = OP_TMP_VARIABLE;
                        tmp->u.var_num = temp_var_num;
                        temp_var_num++;
                        icaddr2->u.triop.result = tmp;
                        place->kind = OP_TMP_ADDRESS;
                        place->u.real_value = tmp;
                    }
                    else
                    {
                        place->kind = OP_TMP_VARIABLE;
                        place->u.var_num = temp_var_num;
                        temp_var_num++;
                    }

                    icaddr2->kind = IC_ADD;
                    insertInterCode(icaddr2);
                }

                Type dbgType = (Type)malloc(sizeof(struct Type_));
                memcpy(dbgType, type->u.array.elem, sizeof(struct Type_));
                dbgType->value = BOTH_VALUE;
                return dbgType;
            }
            else
            {
                printf("Error type 12 at Line %d: \"%s\" is not an integer.\n", exp->child->lineno, exp->child->sibling->sibling->child->nchar_data);
                return NULL;
            }
            break;
        }

        case e_DOT:
        {
            type = Exp(exp->child, place);
            if (type == NULL)
            {
                return NULL;
            }
            if (type->kind != STRUCTURE)
            {
                printf("Error type 13 at Line %d: Illegal use of \".\".\n", exp->child->lineno);
                return NULL;
            }

            fieldList = type->u.structure->tail;
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
            printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", exp->child->lineno, exp->child->sibling->sibling->nchar_data);
            return NULL;
        }

        default:
            break;
        }
    }
    break;

    case e_LP:
        return Exp(exp->child->sibling, place);

    case e_MINUS:
    {
        Operand op1 = (Operand)malloc(sizeof(struct Operand_));
        op1->next = NULL;
        op1->prev = NULL;
        op1->kind = OP_TMP_VARIABLE;
        op1->u.var_num = temp_var_num;
        temp_var_num++;

        type = Exp(exp->child->sibling, op1);
        if (type == NULL)
        {
            return NULL;
        }

        if (type->kind != BASIC)
        {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", exp->child->lineno);
            return NULL;
        }

        type->value = RIGHT_VALUE;

        if (place != NULL)
        {
            if (op1->kind == OP_CONSTANT)
            {
                place->kind = OP_CONSTANT;
                int zk = atoi(op1->u.value);
                zk = 0 - zk;
                sprintf(place->u.value, "%d", zk);
            }
            else
            {
                Operand op0 = (Operand)malloc(sizeof(struct Operand_));
                op0->next = NULL;
                op0->prev = NULL;
                op0->kind = OP_CONSTANT;
                sprintf(op0->u.value, "0");

                interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
                ic1->next = NULL;
                ic1->prev = NULL;
                ic1->kind = IC_SUB;
                ic1->u.triop.op1 = op0;
                ic1->u.triop.op2 = op1;
                ic1->u.triop.result = place;
                insertInterCode(ic1);
            }
        }
    }
        return type;

    case e_NOT:
    {
        Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
        opr1->next = NULL;
        opr1->prev = NULL;
        opr1->kind = OP_LABEL;
        opr1->u.var_num = lbl_num;
        lbl_num++;
        Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
        opr2->next = NULL;
        opr2->prev = NULL;
        opr2->kind = OP_LABEL;
        opr2->u.var_num = lbl_num;
        lbl_num++;

        if (place != NULL)
        {
            interCode ica = (interCode)malloc(sizeof(struct interCode_));
            ica->next = NULL;
            ica->prev = NULL;
            ica->kind = IC_ASSIGN;
            ica->u.binop.left = place;

            Operand opr0 = (Operand)malloc(sizeof(struct Operand_));
            opr0->next = NULL;
            opr0->prev = NULL;
            opr0->kind = OP_CONSTANT;
            sprintf(opr2->u.value, "0");
            ica->u.binop.right = opr0;
            insertInterCode(ica);
        }

        Type type1 = condition_translate(exp->child, opr1, opr2);

        interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
        ic1->next = NULL;
        ic1->prev = NULL;
        ic1->kind = IC_LABEL;
        ic1->u.sinop.op1 = opr1;
        insertInterCode(ic1);

        if (place != NULL)
        {
            interCode ica = (interCode)malloc(sizeof(struct interCode_));
            ica->next = NULL;
            ica->prev = NULL;
            ica->kind = IC_ASSIGN;
            ica->u.binop.left = place;

            Operand opr0 = (Operand)malloc(sizeof(struct Operand_));
            opr0->next = NULL;
            opr0->prev = NULL;
            opr0->kind = OP_CONSTANT;
            sprintf(opr2->u.value, "1");
            ica->u.binop.right = opr0;
            insertInterCode(ica);
        }

        interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
        ic2->next = NULL;
        ic2->prev = NULL;
        ic2->kind = IC_LABEL;
        ic2->u.sinop.op1 = opr2;
        insertInterCode(ic2);
        return type1;
    }
    break;

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
                type = (Type)malloc(sizeof(struct Type_));
                memcpy(type, fieldList->type, sizeof(struct Type_));
                type->value = BOTH_VALUE;

                if (place != NULL)
                {
                    strcpy(place->u.value, exp->child->nchar_data);
                    place->kind = OP_VARIABLE;
                }
                return type;
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
                printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n", exp->child->lineno, exp->child->nchar_data);
                return NULL;
            }
            else
            {
                type = (Type)malloc(sizeof(struct Type_));
                memcpy(type, fieldList->type->u.function.returnType, sizeof(struct Type_));
                type->value = RIGHT_VALUE;

                if (strcmp(fieldList->name, "read") == 0)
                {
                    if (place != NULL)
                    {
                        interCode icr = (interCode)malloc(sizeof(struct interCode_));
                        icr->kind = IC_READ;
                        icr->u.sinop.op1 = place;
                        insertInterCode(icr);
                    }
                }
                else
                {
                    Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
                    opr1->kind = OP_FUNCTION;
                    strcpy(opr1->u.value, fieldList->name);

                    if (place != NULL)
                    {
                        interCode icc = (interCode)malloc(sizeof(struct interCode_));
                        icc->kind = IC_CALL;
                        icc->u.binop.left = place;
                        icc->u.binop.right = opr1;
                        insertInterCode(icc);
                    }
                    else
                    {
                        Operand newplace = (Operand)malloc(sizeof(struct Operand_));
                        newplace->kind = OP_TMP_VARIABLE;
                        newplace->u.var_num = temp_var_num;
                        temp_var_num++;

                        interCode icc = (interCode)malloc(sizeof(struct interCode_));
                        icc->kind = IC_CALL;
                        icc->u.binop.left = newplace;
                        icc->u.binop.right = opr1;
                        insertInterCode(icc);
                    }
                }

                return type;
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
                Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
                opr1->next = NULL;
                opr1->prev = NULL;

                FieldList tmpfl1 = fieldList->type->u.function.params;
                FieldList tmpfl2 = Args(exp->child->sibling->sibling, opr1);

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
                    type = (Type)malloc(sizeof(struct Type_));
                    memcpy(type, fieldList->type->u.function.returnType, sizeof(struct Type_));
                    type->value = RIGHT_VALUE;

                    if (strcmp(fieldList->name, "write") == 0)
                    {
                        interCode icw = (interCode)malloc(sizeof(struct interCode_));
                        icw->kind = IC_WRITE;
                        icw->u.sinop.op1 = opr1->next;
                        insertInterCode(icw);
                        if (place != NULL)
                        {
                            Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
                            opr2->next = NULL;
                            opr2->prev = NULL;
                            opr2->kind = OP_CONSTANT;
                            strcpy(opr2->u.value, "0");

                            interCode ica = (interCode)malloc(sizeof(struct interCode_));
                            ica->kind = IC_ASSIGN;
                            ica->u.binop.left = place;
                            ica->u.binop.right = opr2;
                            insertInterCode(ica);
                        }
                    }
                    else
                    {
                        Operand opr = opr1->next;
                        while (opr != NULL)
                        {
                            interCode icr = (interCode)malloc(sizeof(struct interCode_));
                            icr->kind = IC_ARG;
                            icr->u.sinop.op1 = opr;
                            insertInterCode(icr);
                            opr = opr->next;
                        }

                        Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
                        opr2->next = NULL;
                        opr2->prev = NULL;
                        opr2->kind = OP_FUNCTION;
                        strcpy(opr2->u.value, fieldList->name);

                        if (place != NULL)
                        {
                            interCode icc = (interCode)malloc(sizeof(struct interCode_));
                            icc->kind = IC_CALL;
                            icc->u.binop.left = place;
                            icc->u.binop.right = opr2;
                            insertInterCode(icc);
                        }
                        else
                        {
                            Operand tmpop = (Operand)malloc(sizeof(struct Operand_));
                            tmpop->kind = OP_TMP_VARIABLE;
                            tmpop->u.var_num = temp_var_num;
                            temp_var_num++;

                            interCode icc = (interCode)malloc(sizeof(struct interCode_));
                            icc->kind = IC_CALL;
                            icc->u.binop.left = tmpop;
                            icc->u.binop.right = opr2;
                            icc->next = NULL;
                            icc->prev = NULL;
                            insertInterCode(icc);
                        }
                    }

                    return type;
                }
                else
                {
                    printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n", exp->child->lineno, exp->child->nchar_data);
                    return NULL;
                }
            }
        }
    }
    break;

    case e_INT:
    {
        type = (Type)malloc(sizeof(struct Type_));
        type->kind = BASIC;
        type->u.basic = UBASIC_INT;
        type->value = RIGHT_VALUE;

        if (place != NULL)
        {
            strcpy(place->u.value, exp->child->nchar_data);
            place->kind = OP_CONSTANT;
        }
        return type;
    }

    case e_FLOAT:
    {
        type = (Type)malloc(sizeof(struct Type_));
        type->kind = BASIC;
        type->u.basic = UBASIC_FLOAT;
        type->value = RIGHT_VALUE;

        if (place != NULL)
        {
            strcpy(place->u.value, exp->child->nchar_data);
            place->kind = OP_CONSTANT;
        }
        return type;
    }
    default:
        break;
    }
}

FieldList Args(struct TreeNode *args, Operand place)
{
    FieldList fieldList = (FieldList)malloc(sizeof(struct FieldList_));

    Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
    opr1->kind = OP_TMP_VARIABLE;
    opr1->u.var_num = temp_var_num;
    temp_var_num++;

    fieldList->type = Exp(args->child, opr1);
    fieldList->name = NULL;
    fieldList->tail = NULL;
    opr1->next = place->next;
    place->next = opr1;

    if (args->child->sibling != NULL)
    {
        if (fieldList == NULL)
        {
            fieldList = Args(args->child->sibling->sibling, place);
        }
        else
        {
            FieldList tmp = fieldList;
            while (tmp->tail != NULL)
            {
                tmp = tmp->tail;
            }
            tmp->tail = Args(args->child->sibling->sibling, place);
        }
    }
    return fieldList;
}

Type condition_translate(struct TreeNode *exp, Operand trueop, Operand falseop)
{
    switch (exp->child->int_name)
    {
    case e_Exp:
    {
        switch (exp->child->sibling->int_name)
        {
        case e_AND:
        {
            Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
            opr1->kind = OP_LABEL;
            opr1->u.var_num = lbl_num;
            lbl_num++;
            Type t1 = condition_translate(exp->child, opr1, falseop);

            interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
            ic1->kind = IC_LABEL;
            ic1->u.sinop.op1 = opr1;
            insertInterCode(ic1);

            Type t2 = condition_translate(exp->child->sibling->sibling, trueop, falseop);

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
        }

        case e_OR:
        {
            Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
            opr1->kind = OP_LABEL;
            opr1->u.var_num = lbl_num;
            lbl_num++;
            Type t1 = condition_translate(exp->child, trueop, opr1);

            interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
            ic1->kind = IC_LABEL;
            ic1->u.sinop.op1 = opr1;
            insertInterCode(ic1);

            Type t2 = condition_translate(exp->child->sibling->sibling, trueop, falseop);

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
        }

        case e_RELOP:
        {
            Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
            opr1->kind = OP_TMP_VARIABLE;
            opr1->u.var_num = temp_var_num;
            temp_var_num++;
            Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
            opr2->kind = OP_TMP_VARIABLE;
            opr2->u.var_num = temp_var_num;
            temp_var_num++;
            Type type1 = Exp(exp->child, opr1);
            Type type2 = Exp(exp->child->sibling->sibling, opr2);

            if (type1 == NULL || type2 == NULL)
            {
                return NULL;
            }

            if (type1->kind == BASIC && type2->kind == BASIC && type1->u.basic == type2->u.basic)
            {
                type1->value = RIGHT_VALUE;
                interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
                ic1->kind = IC_IFGOTO;
                ic1->u.ifgotoop.op1 = opr1;
                ic1->u.ifgotoop.op2 = opr2;
                ic1->u.ifgotoop.result = trueop;
                strcpy(ic1->u.ifgotoop.relop, exp->child->sibling->nchar_data);
                insertInterCode(ic1);

                interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
                ic2->kind = IC_GOTO;
                ic2->u.sinop.op1 = falseop;
                insertInterCode(ic2);
                return type1;
            }
            else
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n", exp->child->lineno);
                return NULL;
            }
            break;
        }
        default:
        {
            Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
            opr1->kind = OP_TMP_VARIABLE;
            opr1->u.var_num = temp_var_num;
            temp_var_num++;

            Type type1 = Exp(exp, opr1);

            Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
            opr2->kind = OP_CONSTANT;
            strcpy(opr2->u.value, "0");

            interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
            ic1->kind = IC_IFGOTO;
            ic1->u.ifgotoop.op1 = opr1;
            ic1->u.ifgotoop.op2 = opr2;
            ic1->u.ifgotoop.result = trueop;
            strcpy(ic1->u.ifgotoop.relop, "!=");
            insertInterCode(ic1);

            interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
            ic2->kind = IC_GOTO;
            ic2->u.sinop.op1 = falseop;
            insertInterCode(ic2);
            return type1;
        }
        break;
        }
    }
    break;

    case e_NOT:
    {
        Type t = condition_translate(exp->child->sibling, falseop, trueop);
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
    }

    default:
    {
        Operand opr1 = (Operand)malloc(sizeof(struct Operand_));
        opr1->kind = OP_TMP_VARIABLE;
        opr1->u.var_num = temp_var_num;
        temp_var_num++;

        Type type1 = Exp(exp, opr1);

        Operand opr2 = (Operand)malloc(sizeof(struct Operand_));
        opr2->kind = OP_CONSTANT;
        strcpy(opr2->u.value, "0");

        interCode ic1 = (interCode)malloc(sizeof(struct interCode_));
        ic1->kind = IC_IFGOTO;
        ic1->u.ifgotoop.op1 = opr1;
        ic1->u.ifgotoop.op2 = opr2;
        ic1->u.ifgotoop.result = trueop;
        strcpy(ic1->u.ifgotoop.relop, "!=");
        insertInterCode(ic1);

        interCode ic2 = (interCode)malloc(sizeof(struct interCode_));
        ic2->kind = IC_GOTO;
        ic2->u.sinop.op1 = falseop;
        insertInterCode(ic2);
        return type1;
    }
    }
}

int array_length(Type type, int choose)
{
    if (type == NULL)
    {
        return 0;
    }

    switch (choose)
    {
    case 1:
        if (type->kind == BASIC)
        {
            return 4;
        }
        else
        {
            int basic = 4;
            while (type->u.array.elem->kind != BASIC)
            {
                basic = type->u.array.size * basic;
                type = type->u.array.elem;
            }
            basic = type->u.array.size * basic;
            if (type->u.array.elem->kind == BASIC)
            {
                return basic;
            }
            else
            {
                return 1;
            }
        }

    case 2:
        if (type->u.array.elem->kind == BASIC)
        {
            return 4;
        }
        else
        {
            int basic = 4;
            type = type->u.array.elem;
            while (type->u.array.elem->kind != BASIC)
            {
                basic = type->u.array.size * basic;
                type = type->u.array.elem;
            }
            basic = type->u.array.size * basic;
            if (type->u.array.elem->kind == BASIC)
            {
                return basic;
            }
            else
            {
                return 1;
            }
        }

        break;

    default:
        return 1;
    }

    return 1;
}

int type_check(Type t1, Type t2)
{
    if (t1 == NULL || t2 == NULL)
    {
        return -1;
    }

    if (t1->kind != t2->kind)
    {
        return -1;
    }

    switch (t1->kind)
    {
    case BASIC:
        if (t1->u.basic == t2->u.basic)
        {
            return 0;
        }
        else
        {
            return -1;
        }

    case ARRAY:
        return type_check(t1->u.array.elem, t2->u.array.elem);

    case STRUCTURE:
        if (t1->u.structure->name == NULL && t2->u.structure->name == NULL && t1->noNameStruct == t2->noNameStruct)
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

    case FUNCTION:
        if (type_check(t1->u.function.returnType, t2->u.function.returnType) == 0 && t1->u.function.paramNum == t2->u.function.paramNum)
        {
            FieldList fun1 = t1->u.function.params;
            FieldList fun2 = t2->u.function.params;

            while (fun1 != NULL && fun2 != NULL)
            {
                if (type_check(fun1->type, fun2->type) != 0)
                {
                    return -1;
                }

                fun1 = fun1->tail;
                fun2 = fun2->tail;
            }
            return 0;
        }
        else
        {
            return -1;
        }
    default:
        return 0;
    }
    return 0;
}
