#ifndef inter
#define inter
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int temp_var_num;
extern int var_num;
extern int lbl_num;

typedef struct Operand_ *Operand;

struct Operand_
{
    enum
    {
        OP_VARIABLE, OP_CONSTANT, OP_ADDRESS, OP_TMP_VARIABLE, OP_TMP_ADDRESS, OP_FUNCTION,  OP_LABEL
    } kind;

    union
    {
        int var_num;
        char value[40];
        Operand real_value;
    } u;

    Operand prev, next;
};

typedef struct interCode_ *interCode;
struct interCode_
{
    enum
    {
        IC_LABEL, IC_FUNCTION, IC_ASSIGN, IC_ADD, IC_SUB, IC_MUL, IC_DIV,
        IC_GET_ADDR1, IC_GET_ADDR2, IC_GET_VAR, IC_TO_VAR, IC_GOTO, 
        IC_IFGOTO, IC_RETURN, IC_DEC, IC_ARG, IC_CALL, IC_PARAM, IC_READ, 
        IC_WRITE
    } kind;

    union
    {
        struct {Operand op1;} sinop;
        struct {Operand right,left;} binop;
        struct {Operand result, op1, op2;} triop;
        struct {Operand result, op1, op2;char relop[40];} ifgotoop;
        struct {Operand op1; int size;} decop;
    } u;
    interCode prev, next;
};

void fprintInterCode(char *filename);
void printfOperand(FILE *fp, Operand op);
void insertInterCode(interCode ic);
void deleteInterCodes(interCode ic);

#endif
