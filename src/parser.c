/*
  parser.c
*/

#include "parser.h"
#include "error.h"
#include "optable.h"
#include "string_manip.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* Struct representing a number. It has a value and its type (Depending on  *
 * the value read. Only used by the functions parseNumber and makeOperand.  *
 *                                                                          *
 * Variables:                                                               *
 * val: Value of the number.                                                *
 * type: Type of the number (BYTE1, BYTE2... etc)                           */
typedef struct number_str_s
{
    octa val;
    OperandType type;
} number_str;

/* Returns size of command starting at w (until newline or semicolon)       *
 *                                                                          *
 * Params:                                                                  *
 * c: Pointer to the start of the command                                   *
 *                                                                          *
 * Returns:                                                                 *
 * Size of command fount at given pointer                                   */
int getCommand(const char *c)
{
    int sz = 0;
    for (; *c && *c != '*' && *c != '\n' && *c != ';'; c++)
        sz++;
    return sz;
}

/* Converts from number string (May be hexadecimal - started with # - or    *
 * decimal) to an number structure.                                         *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to string representing (supposedly) a number.                 *
 *                                                                          *
 * Returns:                                                                 *
 * Structure representing a number. Has undefined behavior if w is not a    *
 * number.                                                                  */
number_str parseNumber(const char *w)
{
    number_str val;
    if (isdigit(w[0]) || w[0] == '#' || w[0] == '-')
    {
        if (w[0] == '#')
            sscanf(w, "#%llx", &val.val);
        else
            sscanf(w, "%lld", &val.val);

        if (val.val < 0)
        {
            if ((val.val & 0xffff80) == 0xffff80)
                val.type = BYTE1 | BYTE2 | BYTE3 | TETRABYTE;
            else if ((val.val & 0xff8000) == 0xff8000)
                val.type = BYTE2 | BYTE3 | TETRABYTE;
            else
                val.type = BYTE3 | TETRABYTE;
            val.type |= NEG_NUMBER;
        }
        else
        {
            if (val.val & 0xff0000)
                val.type = BYTE3 | TETRABYTE;
            else if (val.val & 0x00ff00)
                val.type = BYTE2 | BYTE3 | TETRABYTE;
            else
                val.type = BYTE1 | BYTE2 | BYTE3 | TETRABYTE;
        }
    }
    return val;
}

/* Receives string and returns the type of the operand read.                *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to a null-terminated string containing the operand string to  *
 * be evaluated.                                                            *
 *                                                                          *
 * Returns:                                                                 *
 * 0 if w is null, REGISTER if the string is a register, NUMBER_TYPE if it  *
 * is a number, STRING if it's a quoted or double-quotes delimitated string *
 * , and LABEL if it is a possible label. Returns -1 if it is not any.      */
OperandType operandType(const char *w)
{
    if (!w)
        return 0;

    int wlen = strlen(w);

    if (w[0] == '$')
    {
        if (operandType(w + 1) & BYTE1)
            return REGISTER;
        else
            return -1;
    }

    if (w[0] == '#')
    {
        for (int i = 1; i < wlen; i++)
        {
            if (!isxdigit(w[i]))
                return -1;
        }
        return parseNumber(w).type;
    }
    else if (isdigit(w[0]) || w[0] == '-')
    {
        for (int i = 1; i < wlen; i++)
        {
            if (!isdigit(w[i]))
                return -1;
        }
        return parseNumber(w).type;
    }

    if (w[0] == '"')
    {
        if (w[wlen - 1] != '"')
            return -1;
        for (int i = 1; i < wlen - 1; i++)
        {
            if (w[i] == '\\' && !w[wlen - 2])
                return -1;
            if (w[i] == '"' && w[i - 1] != '\\')
                return -1;
        }
        return STRING;
    }

    if (isalpha(w[0]) || w[0] == '_')
    {
        for (int i = 1; i < wlen; i++)
        {
            if (!isalnum(w[i]) && w[i] != '_')
                return -1;
        }
        return LABEL;
    }
    return -1;
}

/* Creates new operand object from string given to it. Returns 0 in case of *
 * error. (String doesn't represent any operand)                            *
 *                                                                          *
 * Params:                                                                  *
 * word: Pointer to the start of the operand string to make into an object. *
 *                                                                          *
 * Returns:                                                                 *
 * New operand object. Returns 0 if it couldn't be converted.               */
Operand *makeOperand(const char *word)
{
    if (optable_find(word))
        return 0;

    unsigned int type = operandType(word);

    switch (type)
    {
    case REGISTER:
    {
        number_str number = parseNumber(word + 1);
        if (!(number.type & NEG_NUMBER) && (number.type & BYTE1))
            return operand_create_register(number.val);
        break;
    }
    case LABEL:
    {
        return operand_create_label(word);
        break;
    }
    case STRING:
        return operand_create_string(word);
        break;
    default:
        break;
    }

    if (type & NUMBER_TYPE)
    {
        number_str number = parseNumber(word);
        return operand_create_number(number.val);
    }
    return 0;
}

Instruction *parseCommand(const char *command, int sz, SymbolTable alias_table, const char **errptr)
{
    char *label = 0;
    const Operator *operator= 0;
    Operand *opds[3];
    for (int i = 0; i < 3; i++)
        opds[i] = 0;

    int wl;
    const char *empty = nextWord(command, ';', 0);
    if (empty)
        return 0;

    const char *current = nextWord(command, 0, errptr);
    if (errptr && *errptr)
    {
        //DID NOT EXPECT
        set_error_msg("Unexpected character");
        return 0;
    }

    // First word
    const char *first_w = readWord(current, &wl);
    operator= optable_find(first_w);

    if (!operator)
    {
        label = (char *)first_w;
        if (stable_find(alias_table, first_w))
        {
            //ALREADY IN TABLE
            set_error_msg("Alias already declared");
            free((char *)first_w);
            return 0;
        }
    }

    current += wl;
    current = nextWord(current, 0, errptr);
    if (errptr && *errptr)
    {
        //DID NOT EXPECT
        set_error_msg("Unexpected character");
        free((char *)first_w);
        return 0;
    }

    // Second word
    if (label)
    {
        const char *second_w = readWord(current, &wl);
        operator= optable_find(second_w);
        if (!operator)
        {
            //NO OPERATOR
            set_error_msg("No operator found");
            free((char *)first_w);
            free((char *)second_w);
            if (errptr)
                *errptr = current;
            return 0;
        }

        if (operator->opcode == EXTERN)
        {
            //LABELED EXTERN
            set_error_msg("EXTERN cannot have a label");
            free((char *)first_w);
            free((char *)second_w);
            if (errptr)
                *errptr = current;
            return 0;
        }

        current += wl;
        current = nextWord(current, 0, errptr);
        if (errptr && *errptr)
        {
            //DID NOT EXPECT
            set_error_msg("Unexpected character");
            free((char *)first_w);
            free((char *)second_w);
            if (errptr)
                *errptr = current;
            return 0;
        }

        free((char *)second_w);
    }

    // Remaining words
    int i;
    for (i = 0; i < 3 && operator->opd_types[i] != OP_NONE && current; i++)
    {
        const char *word = readWord(current, &wl);
        Operand *opd = 0;
        OperandType optype = operandType(word);
        if (optype == LABEL && !(operator->opd_types[i] &LABEL))
        {
            EntryData *data;
            if (!(data = stable_find(alias_table, word)))
            {
                //LABEL DOES NOT EXIST
                set_error_msg("Label not declared");
                free((char *)first_w);
                free((char *)word);
                if (errptr)
                    *errptr = current;
                return 0;
            }
            optype = data->opd->type;
            opd = operand_dup(data->opd);
        }

        if (!(optype & operator->opd_types[i]))
        {
            //WRONG OPERAND TYPE
            set_error_msg("Wrong operand type");
            free((char *)first_w);
            free((char *)word);
            if (errptr)
                *errptr = current;
            return 0;
        }

        if (!opd)
            opd = makeOperand(word);

        opds[i] = opd;
        free((char *)word);

        current += wl;
        if (i < 2 && operator->opd_types[i + 1] != OP_NONE)
            current = nextWord(current, ',', errptr);
    }

    if (i < 2 && operator->opd_types[i + 1] != OP_NONE)
    {
        //EXPECTED OPERAND
        set_error_msg("Too few operands for this operator");
        free((char *)first_w);
        for (int j = 0; j < 3; j++)
            free(opds[i]);
        if (errptr)
            *errptr = current;
        return 0;
    }

    if (operator->opcode == IS)
    {
        InsertionResult res = stable_insert(alias_table, label);
        res.data->opd = opds[0];
    }

    Instruction *instruction = instr_create(label, operator, opds);
    free(label);
    return instruction;
}

int parse(const char *s, SymbolTable alias_table, Instruction **instr,
          const char **errptr)
{
    *errptr = 0;
    const char *next = s;
    next = nextWord(s, 0, 0);
    Instruction *last = *instr;
    if (!stable_find(alias_table, "rA"))
        stable_insert(alias_table, "rA").data->opd = operand_create_register(255);
    if (!stable_find(alias_table, "rR"))
        stable_insert(alias_table, "rR").data->opd = operand_create_register(254);
    if (!stable_find(alias_table, "rSP"))
        stable_insert(alias_table, "rSP").data->opd = operand_create_register(253);
    if (!stable_find(alias_table, "rX"))
        stable_insert(alias_table, "rX").data->opd = operand_create_register(252);
    if (!stable_find(alias_table, "rY"))
        stable_insert(alias_table, "rY").data->opd = operand_create_register(251);
    if (!stable_find(alias_table, "rZ"))
        stable_insert(alias_table, "rZ").data->opd = operand_create_register(250);
    do
    {
        for (; last && last->next; last = last->next)
            ;
        if (next)
        {
            int command_len = getCommand(next);
            Instruction *tmp = 0;
            if (command_len)
                tmp = parseCommand(next, command_len, alias_table, errptr);
            if (errptr && *errptr)
            {
                if (**errptr == '*' || **errptr == '\n')
                {
                    *errptr = 0;
                    break;
                }
                printf("ERROR: %s\n", *errptr);
            }
            if (errptr && *errptr)
                return 0;

            if (!last)
            {
                last = *instr = tmp;
                if (last)
                {
                    if (last->op->opcode != IS && last->op->opcode != EXTERN)
                        last->lineno = 0;
                    else
                        last->lineno = -1;
                }
            }
            else
            {
                last->next = tmp;
                if (last->next)
                {
                    if (last->next->op->opcode != IS && last->op->opcode != EXTERN)
                        last->next->lineno = last->lineno + 1;
                    else
                        last->next->lineno = last->lineno;
                }
            }
            next += command_len;
        }

        next = nextWord(next, ';', errptr);
        if (errptr && *errptr)
        {
            if (**errptr == '*' || **errptr == '\n')
            {
                *errptr = 0;
                return 1;
            }
            return 0;
        }
    } while (next);

    return 1;
}
