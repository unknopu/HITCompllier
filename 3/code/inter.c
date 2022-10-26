#include "inter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int var_num = 1;
int lbl_num = 1;
int temp_var_num = 1;

interCode InterCode = NULL;
interCode InterCodeTail = NULL;

void insertInterCode(interCode ic)
{
    if (InterCode == NULL){
        ic->next = NULL;
        ic->prev = NULL;
        InterCode = ic;
        InterCodeTail = ic;
    }
    else{
        ic->prev = InterCodeTail;
        ic->next = NULL;
        InterCodeTail->next = ic;
        InterCodeTail = ic;
    }
}

void fprintInterCode(char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        return;
    }

    interCode currentCode = InterCode;

    while (currentCode != NULL)
    {
         (currentCode->kind)
        {
        case IC_LABEL:
            fprintf(fp, "LABEL ");
            printfOperand(fp, currentCode->u.sinop.op1);
            fprintf(fp, " : ");
            break;

        case IC_FUNCTION:switch
            fprintf(fp, "FUNCTION ");
            printfOperand(fp, currentCode->u.sinop.op1);
            fprintf(fp, " : ");
            break;

        case IC_ASSIGN:
            printfOperand(fp, currentCode->u.binop.left);
            fprintf(fp, " := ");
            printfOperand(fp, currentCode->u.binop.right);
            break;

        case IC_ADD:
            printfOperand(fp, currentCode->u.triop.result);
            fprintf(fp, " := ");
            printfOperand(fp, currentCode->u.triop.op1);
            fprintf(fp, " + ");
            printfOperand(fp, currentCode->u.triop.op2);
            break;

        case IC_SUB:
            printfOperand(fp, currentCode->u.triop.result);
            fprintf(fp, " := ");
            printfOperand(fp, currentCode->u.triop.op1);
            fprintf(fp, " - ");
            printfOperand(fp, currentCode->u.triop.op2);
            break;

        case IC_MUL:
            printfOperand(fp, currentCode->u.triop.result);
            fprintf(fp, " := ");
            printfOperand(fp, currentCode->u.triop.op1);
            fprintf(fp, " * ");
            printfOperand(fp, currentCode->u.triop.op2);
            break;

        case IC_DIV:
            printfOperand(fp, currentCode->u.triop.result);
            fprintf(fp, " := ");
            printfOperand(fp, currentCode->u.triop.op1);
            fprintf(fp, " / ");
            printfOperand(fp, currentCode->u.triop.op2);
            break;

        case IC_GET_ADDR1:
            printfOperand(fp, currentCode->u.binop.left);
            fprintf(fp, " := &");
            printfOperand(fp, currentCode->u.binop.right);
            break;

        case IC_GET_ADDR2:
            printfOperand(fp, currentCode->u.triop.result);
            fprintf(fp, " := &");
            printfOperand(fp, currentCode->u.triop.op1);
            fprintf(fp, " + ");
            printfOperand(fp, currentCode->u.triop.op2);
            break;

        case IC_GET_VAR:
            printfOperand(fp, currentCode->u.binop.left);
            fprintf(fp, " := *");
            printfOperand(fp, currentCode->u.binop.right);
            break;

        case IC_TO_VAR:
            fprintf(fp, "*");
            printfOperand(fp, currentCode->u.binop.left);
            fprintf(fp, " := ");
            printfOperand(fp, currentCode->u.binop.right);
            break;

        case IC_GOTO:
            fprintf(fp, "GOTO ");
            printfOperand(fp, currentCode->u.sinop.op1);
            break;

        case IC_IFGOTO:
            fprintf(fp, "IF ");
            printfOperand(fp, currentCode->u.ifgotoop.op1);
            fprintf(fp, " %s ", currentCode->u.ifgotoop.relop);
            printfOperand(fp, currentCode->u.ifgotoop.op2);
            fprintf(fp, " GOTO ");
            printfOperand(fp, currentCode->u.ifgotoop.result);
            break;

        case IC_RETURN:
            fprintf(fp, "RETURN ");
            printfOperand(fp, currentCode->u.sinop.op1);
            break;

        case IC_DEC:
            fprintf(fp, "DEC ");
            printfOperand(fp, currentCode->u.decop.op1);
            fprintf(fp, " %d ", currentCode->u.decop.size);
            break;

        case IC_ARG:
            fprintf(fp, "ARG ");
            printfOperand(fp, currentCode->u.sinop.op1);
            break;

        case IC_CALL:
            printfOperand(fp, currentCode->u.binop.left);
            fprintf(fp, " := CALL ");
            printfOperand(fp, currentCode->u.binop.right);
            break;

        case IC_PARAM:
            fprintf(fp, "PARAM ");
            printfOperand(fp, currentCode->u.sinop.op1);
            break;

        case IC_READ:
            fprintf(fp, "READ ");
            printfOperand(fp, currentCode->u.sinop.op1);
            break;

        case IC_WRITE:
            fprintf(fp, "WRITE ");
            printfOperand(fp, currentCode->u.sinop.op1);
            break;

        default:
            break;
        }
        fprintf(fp, "\n");
        currentCode = currentCode->next;
    }
    fclose(fp);
}

void printfOperand(FILE *fp, struct Operand_ *op)
{
    if (fp == NULL || op == NULL)
    {
        return;
    }

    switch (op->kind)
    {
    case OP_VARIABLE:
        fprintf(fp, "v%s", op->u.value);
        break;

    case OP_CONSTANT:
        fprintf(fp, "#%s", op->u.value);
        break;

    case OP_ADDRESS:
        fprintf(fp, "*v%s", op->u.real_value->u.value);
        break;

    case OP_TMP_VARIABLE:
        fprintf(fp, "t%d", op->u.var_num);
        break;

    case OP_TMP_ADDRESS:
        fprintf(fp, "*t%d", op->u.real_value->u.var_num);
        break;

    case OP_FUNCTION:
        fprintf(fp, "%s", op->u.value);
        break;

    case OP_LABEL:
        fprintf(fp, "label%d", op->u.var_num);
        break;
        
    default:
        break;
    }
}

void deleteInterCodes(interCode ic)
{
    if (ic == InterCode)
    {
        InterCode=InterCode->next;
        InterCode->prev=NULL;
    }
    else if (ic == InterCodeTail)
    {
        InterCodeTail=InterCodeTail->prev;
        InterCodeTail->next=NULL;
    }
    else
    {
        ic->prev->next = ic->next;
        ic->next->prev = ic->prev;
    }
}
