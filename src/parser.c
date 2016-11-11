/*
  parser.c
*/

#include "parser.h"
#include "error.h"
#include "optable.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char byte;

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
 * Size of command fount at given pointer                                  */
int getCommand(const char *c)
{
    int sz = 0;
    for (; *c && *c != '*' && *c != '\n' && *c != ';'; c++)
        sz++;
    return sz;
}

/* Returns the size of a word starting at w (until space, newline, comma or *
 * semicolon). Exception - If w[0] is ", then read until an unescaped ".    *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the word                                      *
 *                                                                          *
 * Returns:                                                                 *
 * Size of word found at given pointer                                      */
int getWord(const char *w)
{
    int sz = 0;
    byte is_str = 0;
    if (w[0] == '"')
    {
        is_str = 1;
        w++;
    }
    for (; *w && ((*w != '*' && !isspace(*w) && *w != ',' && *w != ';')
                || (is_str && (*w != '"' || *(w - 1) == '\\'))); w++)
        sz++;
    return sz;
}

/* Returns the pointer to the start of the next word, checking for expected *
 * characters and ignoring spaces.                                          *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the search                                    *
 * expect: If this char doesn't appear until the start of the next word, it *
 * sets *error as the start of the next word and returns 0.                 *
 *                                                                          *
 * Returns:                                                                 *
 * Pointer to the start of the next word, zero if error or if EOL.          */
const char *nextWord(const char *w, char expect, const char **error)
{
    const char *ptr = w;
    byte found = 0;
    for (; *ptr && *ptr != '*' && (isspace(*ptr) || *ptr == ',' || *ptr == ';');
         ptr++)
    {
        if (*ptr == expect)
            found = 1;
    }

    if (!found && expect)
    {
        if(error) *error = (char*) ptr;
        return 0;
    }

    if (!*ptr)
    {
        if(error) *error = 0;
        ptr = 0;
    }
    return ptr;
}

/* Reads word starting at w to a separate string. If wl is given, its value *
 * is set to that of the length of the word that was read.                  *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the search                                    *
 * wl: Pointer to an integer to put the size of the word into. (Optional)   *
 *                                                                          *
 * Returns:                                                                 *
 * Pointer to the location of the newly allocated word.                     */
const char *readWord(const char *w, int *wl)
{
    if (!w)
        return 0;
    int word_len = getWord(w);

    if (wl)
        *wl = word_len;
    char *word = emalloc((word_len + 1) * sizeof(char));
    for (int i = 0; i <= word_len; i++)
        word[i] = w[i];

    word[word_len] = 0;
    return word;
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
    if (isdigit(w[0]) || w[0] == '#')
    {
        if (w[0] == '#')
            sscanf(w, "#%llx", &val.val);
        else
            sscanf(w, "%lld", &val.val);

        if (val.val < 0)
        {
            if ((val.val & 0xffff80) == 0xffff80)
                val.type = BYTE1;
            else if ((val.val & 0xff8000) == 0xff8000)
                val.type = BYTE2;
            else val.type = BYTE3;
            val.type |= NEG_NUMBER;
        } else {
            if (val.val & 0xff0000)
                val.type = BYTE3;
            else if (val.val & 0x00ff00)
                val.type = BYTE2;
            else val.type = BYTE1;
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
        if (operandType(w + 1) == NUMBER_TYPE)
            return REGISTER;
        else return -1;
    }

    if (w[0] == '#')
    {
        for (int i = 1; i < wlen; i++)
        {
            if (!isxdigit(w[i]))
                return -1;
        }
        return NUMBER_TYPE;
    } else if (isdigit(w[0])){
        for (int i = 1; i < wlen; i++)
        {
            if (!isdigit(w[i]))
                return -1;
        }
        return NUMBER_TYPE;
    }

    if (w[0] == '"')
    {
        if (w[wlen - 1] != '"')
            return -1;
        for (int i = 1; i < wlen - 1; i++)
        {
            if (w[i] == '\\' && !w[wlen-2])
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
    switch (operandType(word))
    {
    case REGISTER:
    {
        number_str number = parseNumber(word + 1);
        return operand_create_register(number.val);
        break;
    }
    case NUMBER_TYPE:
    {
        number_str number = parseNumber(word);
        return operand_create_number(number.val);
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
    return 0;
}

Instruction *parseCommand(const char *command, int sz, SymbolTable alias_table, const char **errptr)
{
    // REDOING
    char *label = 0;
    const Operator * operator = 0;
    Operand *opds[3];
    for (int i = 0; i < 3; i++) opds[i] = 0;

    int wl;
    const char *current = nextWord(command, 0, errptr);
    if (errptr && *errptr)
    {
        //DID NOT EXPECT
        return 0;
    }

    // First word
    const char *first_w = readWord(current, &wl);
    current += wl;
    operator = optable_find(first_w);

    if (!operator)
    {
        label = (char*) first_w;
        if (stable_find(alias_table, first_w))
        {
            //ALREADY IN TABLE
            free((char*)first_w);
            return 0;
        }
    }

    current = nextWord(current, 0, errptr);
    if (errptr && *errptr)
    {
        //DID NOT EXPECT
        free((char*)first_w);
        return 0;
    }

    // Second word
    if (label)
    {
        const char *second_w = readWord(current, &wl);
        current += wl;
        operator = optable_find(second_w);
        if (!operator)
        {
            //NO OPERATOR
            free((char*)first_w);
            free((char*)second_w);
            if (errptr)
                *errptr = current;
            return 0;
        }

        current = nextWord(current, 0, errptr);
        if (errptr && *errptr)
        {
            //DID NOT EXPECT
            free((char*)first_w);
            free((char*)second_w);
            if (errptr)
                *errptr = current;
            return 0;
        }

        free((char*)second_w);
    }

    // Remaining words
    int i;
    for (i = 0; i < 3 && operator->opd_types[i] != OP_NONE && current; i++)
    {
        const char *word = readWord(current, &wl);
        current += wl;
        Operand *opd = 0;
        OperandType optype = operandType(word);
        if (optype == LABEL)
        {
            EntryData *data;
            if (!(data = stable_find(alias_table, word)))
            {
                //LABEL DOES NOT EXIST
                free((char*)first_w);
                free((char*)word);
                if (errptr)
                    *errptr = current;
                return 0;
            }
            optype = data->opd->type;
            opd = data->opd;
        }

        if (!(optype & operator->opd_types[i]))
        {
            //WRONG OPERAND TYPE
            free((char*)first_w);
            free((char*)word);
            if (errptr)
                *errptr = current;
            return 0;
        }

        if (!opd)
            opd = makeOperand(word);

        opds[i] = opd;
        free((char*)word);
        if (i < 2 && operator->opd_types[i + 1] != OP_NONE)
            current = nextWord(current, ',', errptr);
    }

    if (i < 2 && operator->opd_types[i + 1] != OP_NONE)
    {
        //EXPECTED OPERAND
        free((char*)first_w);
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
    const char *next = s;
    int instruction_index = 0;
    next = nextWord(s, 0, 0);
    do
    {
        if (next)
        {
            int command_len = getCommand(next);
            instr[instruction_index++] = parseCommand(next, command_len, alias_table, errptr);
            next += command_len;
        }
        next = nextWord(next,';', errptr);
        if (errptr && *errptr)
        {
            if (**errptr == '*')
            {
                *errptr = 0;
                return 1;
            }
        }
    } while (next);

    return 1;
}
