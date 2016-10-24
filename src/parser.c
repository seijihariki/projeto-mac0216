/*
  parser.c
*/

#include "parser.h"
#include "error.h"
#include "optable.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
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
 * semicolon).                                                              *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the word                                      *
 *                                                                          *
 * Returns:                                                                 *
 * Size of word found at given pointer                                      */
int getWord(const char *w)
{
    int sz = 0;
    for (; *w && *w != '*' && !isspace(*w) && *w != ',' && *w != ';'; w++)
        sz++;
    return sz;
}

/* Returns the pointer to the start of the next word, ignoring spaces,      *
 * newlines, commas, and semicolons                                         *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the search                                    *
 *                                                                          *
 * Returns:                                                                 *
 * Pointer to the start of the next word                                    */
const char *nextWord(const char *w)
{
    const char *ptr = w;
    for (; *ptr && *ptr != '*' &&
           (isspace(*ptr) || *ptr == ',' || *ptr == ';');
         ptr++)
        ;

    if (!*ptr)
        ptr = 0;
    return ptr;
}

/* Returns the pointer to the start of the next word, ignoring spaces,      *
 * newlines, commas, and semicolons                                         *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the search                                    *
 *                                                                          *
 * Returns:                                                                 *
 * Pointer to the start of the next word                                    */
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

int operandType(const char *w)
{
    if (!w)
        return 0;
    if (w[0] == '$')
        return REGISTER;
    if (isdigit(w[0]) || (w[0] == 'h' && isdigit(w[1])))
        return NUMBER_TYPE;
    if (w[0] == '"')
        return STRING;
    return LABEL;
}

Operand *makeOperand(const char *word, int word_len)
{
    switch (operandType(word))
    {
    case REGISTER:
    {
        unsigned int reg;
        sscanf(word, "$%u", &reg);
        return operand_create_register(reg);
        break;
    }
    case NUMBER_TYPE:
    {
        octa num;
        if (word[0] == 'h')
            sscanf(word, "h%llx", &num);
        else
            sscanf(word, "%lld", &num);
        return operand_create_number(num);
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

Instruction *parseCommand(const char *command, int sz)
{
    printf("%s\n", command);
    Instruction *instruction = (Instruction *)emalloc(sizeof(struct instruction_s));

    const Operator *op;

    // First word
    int word_len;
    const char *curr = nextWord(command);
    const char *word = readWord(curr, &word_len);

    instruction->label = 0;
    if (!(op = optable_find(word)))
    {
        if (operandType(word) == LABEL)
        {
            instruction->label = (char *)emalloc((word_len + 1) * sizeof(char));
            strcpy(instruction->label, word);
        }
        else
        {
            printf("PROBLEMAS 1a palavra nao e operador nem label\n");
        }
    }
    free((char *)word);
    curr += word_len;

    // Second word, if first word wasn't operator
    curr = nextWord(curr);
    word = readWord(curr, &word_len);

    if (!op)
    {
        op = optable_find(word);

        free((char *)word);
        curr += word_len;
        curr = nextWord(curr);
        word = readWord(curr, &word_len);
    }
    if (op)
        instruction->op = op;
    else
    {
        // PROBLEMAS Nenhuma das palavras anteriores é operador
    }

    // Remaining words
    for (int i = 0; i < 3 && curr && curr < command + sz; i++)
    {
        instruction->opds[i] = makeOperand(word, word_len);

        free((char *)word);
        curr += word_len;
        curr = nextWord(curr);
        word = readWord(curr, &word_len);
    }

    free((char *)word);

    return instruction;
}

int parse(const char *s, SymbolTable alias_table, Instruction **instr,
          const char **errptr)
{
    const char *next = s;
    int instruction_index = 0;
    do
    {
        next = nextWord(next);
        if (next)
        {
            int command_len = getCommand(next);
            instr[instruction_index++] = parseCommand(next, command_len);
            next += command_len;
        }
    } while (next);

    return 1;
}
