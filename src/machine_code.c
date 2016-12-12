#include "machine_code.h"

#include <stdlib.h>

void append_instr (Instr **instr_chain, Instr *instruction)
{
    if (!instr_chain)
        return;
    if (*instr_chain)
    {
        Instr *curr = *instr_chain;
        while (curr->next)
            curr = curr->next;
        curr->next = instruction;
    } else {
        *instr_chain = instruction;
    }
    return;
}

void free_instr_chain (Instr **instr_chain)
{
    if (!instr_chain)
        return;
    if (*instr_chain)
    {
        Instr *curr = *instr_chain, *bef = 0;
        while (curr->next)
        {
            bef = curr;
            curr = curr->next;
            if (curr->opcode)
                free(curr->data.alias);
            free (bef);
        }
    }
    *instr_chain = 0;
}

Instr* create_instr_i (char* alias, unsigned char opcode, unsigned char reg)
{
    if (!alias)
        return 0;
    Instr* ret = malloc(sizeof(struct instr_s));
    if (!ret)
        return 0;
    ret->data.alias = malloc((strlen(alias) + 1)*sizeof(char));
    if (!ret->data.alias)
        return 0;
    strcpy(ret->data.alias, alias);
    ret->opcode = opcode;
    ret->next = 0;
    ret->reg = reg;
    return ret;
}

Instr* create_instr_d (unsigned int code)
{
    Instr* ret = malloc(sizeof(struct instr_s));
    if (!ret) return 0;
    ret->data.code = code;
    ret->reg = 0;
    ret->opcode = 0;
    ret->next = 0;
    return ret;
}
