#include "buffer.h"
#include "error.h"
#include "parser.h"
#include "stable.h"
#include "opcodes.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void instructions_destroy(Instruction **instr_list)
{
    Instruction *current = *instr_list, *last = 0;
    while (current)
    {
        last = current;
        current = current->next;
        instr_destroy(last);
    }
}

/* Deletes a given item from a given entry data.                            *
 *                                                                          *
 * Params:                                                                  *
 * key: String key of the data.                                             *
 * data: Entry to be deleted from table.                                    */
int del_item(const char *key, EntryData *data)
{
    if (data->opd)
        free(data->opd);
    return 1;
}

/* Modifies the file extension to .maco for the generated file.             *
 *                                                                          *
 * Params:                                                                  *
 * input_file: Name of the input file.                                      */
char* objfile_name(char* input_file)
{
    int orlen = strlen(input_file);
    char* ret = malloc((orlen + 6)*sizeof(char));
    int i;
    for (i = 0; i < orlen && input_file[i] != '.' && input_file[i]; i++)
        ret[i] = input_file[i];
    strcpy(&ret[i], ".maco");
    return ret;
}

/* Struct representing a node of a linked list of instructions. It can be a *
 * defined (Already in machine code) or undefined (For jumps to other       *
 * file's labels, in the format '* <opcode> <label>'). The order of the     *
 * list is the order it ultimately is written to the output file.           */
typedef struct instr_s Instr;

struct instr_s {
    union {
        unsigned int code;
        char* alias;
    } data;
    unsigned char reg;
    unsigned char opcode;
    Instr* next;
};

/* Appends a given instruction to a given linked list of instructions.      *
 *                                                                          *
 * Params:                                                                  *
 * instr_chain: Chain (linked list) of instructions to append to            *
 * instruction: Instruction to append to linked list                        */
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

/* Frees all objects of a given linked list of instructions, along with     *
 * strings of undefined instructions.                                       *
 *                                                                          *
 * Params:                                                                  *
 * instr_chain: Chain (linked list) of instructions to free                 */
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

/* Creates a undefined instruction from a given alias and opcode read from  *
 * the object file.                                                         *
 *                                                                          *
 * Params:                                                                  *
 * alias: Alias read from object file                                       *
 * opcode: Opcode read from object file                                     *
 *                                                                          *
 * Returns pointer to new instruction. 0 if an error occurred.              */
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

/* Creates a defined instruction from a given hexadecimal value read from   *
 * the object file.                                                         *
 *                                                                          *
 * Params:                                                                  *
 * code: The instruction, read as an hexadecimal value from object file     *
 *                                                                          *
 * Returns pointer to new instruction. 0 if an error occurred.              */
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


/* Handles some escaped characters like \n and \"                           *
 *                                                                          *
 * Params:                                                                  *
 * output: Output string where the result will be put into.                 *
 * input: Input string that will be varified.                               */
void unescape(char* output, char* input)
{
    char *curr_i = input;
    char *curr_o = output;

    while (*curr_i)
    {
        char out_char = 0;
        if (*curr_i == '\\')
        {
            switch (*(curr_i + 1))
            {
                case 'n':
                    out_char = 0xa;
                    break;
                case '"':
                    out_char = '"';
                    break;
                default:
                    *(curr_o++) = '\\';
                    out_char = *(curr_i + 1);
                    break;
            }
            curr_i += 2;
            *(curr_o++) = out_char;
        }
        else if (*curr_i == '"')
            curr_i++;
        else {
            out_char = *(curr_i++);
            *(curr_o++) = out_char;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        exit(-1);
    }

    // Initialize all
    set_prog_name("macas");

    SymbolTable alias_table = stable_create();
    SymbolTable label_table = stable_create();

    FILE *file = fopen(argv[1], "ro");
    char* objfile = objfile_name(argv[1]);
    FILE *outfile = fopen(objfile, "wo");

    Instr *compiled = 0;

    free(objfile);
    Buffer *line = buffer_create();
    Instruction *current = 0;
    Instruction *init = 0;

    const char *errptr = 0;

    // Creates linked list of instructions, handling errors when necessary
    while (read_line(file, line))
    {
        if (!parse(line->data, alias_table, &init, &errptr))
        {
            printf("%s", line->data);
            for (int i = 0; i < (unsigned int)((errptr - line->data) / sizeof(char)); i++)
                printf((line->data[i] == '\t') ? "\t" : " ");
            printf("^\n");
            die(0);
        }
    }

    Instruction *ext_check = 0;
    for (current = init; current; current = current->next)
    {
        if (current->op->opcode == EXTERN)
        {
            int ok = 0;
            for (ext_check = current->next; ext_check; ext_check = ext_check->next)
            {
                if (ext_check->label && !strcmp(ext_check->label, current->opds[0]->value.label))
                {
                    if (ext_check->op->opcode != IS)
                        ok = 1;
                    else
                        die("Extern cannot point to IS instruction\n");
                }
            }
            if (!ok)
                die("Extern label %s never defined\n", current->opds[0]->value.label);
        }
    }

    // Creates SymbolTable assigning line numbers to line labels and fills
    // machine code linked list
    int curr_instr = 0;
    for (current = init; current; current = current->next)
    {
        if (current->label)
        {
            InsertionResult res = stable_insert(label_table, current->label);
            if (res.new)
                res.data->i = curr_instr;
        }

        if (current->op->opcode == IS || current->op->opcode == EXTERN)
            continue;

        switch (current->op->opcode)
        {
            case STR:
                {
                    char *cursor = malloc(strlen(current->opds[0]->value.str) * sizeof(char));
                    int str_num = 0x00000000;
                    Instr *str_part;
                    unescape(cursor, current->opds[0]->value.str);
                    printf("%s",cursor);
                    while (*cursor || (str_num & 0xff))
                    {
                        str_num = 0x00000000;
                        for (int i = 0; i < 4; i++)
                            str_num |= *(cursor++) << 8*(3 - i);
                        printf("%08X\n", str_num);
                        str_part = create_instr_d(str_num);
                        append_instr(&compiled, str_part);
                        curr_instr++;
                    }
                    break;
                }
            case PUSH:
                {
                    Instr *machine_code = create_instr_d(
                            0x1f00fd00 +
                            (current->opds[0]->value.reg << 16));
                    append_instr(&compiled, machine_code);

                    machine_code = create_instr_d(0x31fdfd08);
                    append_instr(&compiled, machine_code);

                    curr_instr += 2;
                    break;
                }
            case CALL:
                {
                    Instr *machine_code = create_instr_d(0x58fa0004);
                    append_instr(&compiled, machine_code);

                    machine_code = create_instr_d(0x1ffafd00);
                    append_instr(&compiled, machine_code);

                    machine_code = create_instr_d(0x31fdfd08);
                    append_instr(&compiled, machine_code);

                    machine_code = create_instr_i(current->opds[0]->value.label, 0x48, 0);
                    append_instr(&compiled, machine_code);

                    curr_instr += 4;
                    break;
                }
            case RET:
                {
                    Instr *machine_code;

                    machine_code = create_instr_d(0x33fdfd08 + 8*current->opds[0]->value.num);
                    append_instr(&compiled, machine_code);

                    machine_code = create_instr_d(0x0ffafd00 + 8*current->opds[0]->value.num);
                    append_instr(&compiled, machine_code);

                    machine_code = create_instr_d(0x56fa0000);
                    append_instr(&compiled, machine_code);
                    curr_instr += 3;
                    break;

                }
            case JMP:
                {
                    Instr *machine_code;
                    if (current->opds[0]->type & (BYTE3 | NEG_NUMBER))
                    {
                        octa value = current->opds[0]->value.num;
                        int operator = JMP;
                        if (current->opds[0]->type == NEG_NUMBER)
                        {
                            operator++;
                            value = -value;
                        } else {
                            Instruction *curr_t;
                            int cnt = 0;
                            int finval = 0;
                            for (curr_t = current->next; curr_t && cnt < value - 1; cnt++, curr_t = curr_t->next)
                            {
                                if (curr_t->op->opcode == CALL)
                                    finval += 4;
                                else if (curr_t->op->opcode == PUSH)
                                    finval += 2;
                                else if (curr_t->op->opcode == RET)
                                    finval += 3;
                                else
                                    finval++;
                            }
                        }
                        machine_code = create_instr_d((operator << 24) + (0xffffff & value));
                    } else
                        machine_code = create_instr_i(current->opds[0]->value.label, 0x48, 0);
                    append_instr(&compiled, machine_code);
                    curr_instr ++;
                    break;
                }
            case JZ:
            case JNZ:
            case JP:
            case JN:
            case JNN:
            case GETA:
            case JNP:
                {
                    Instr *machine_code;
                    int operator = current->op->opcode;
                    if (current->opds[1]->type & (BYTE2 | NEG_NUMBER))
                    {
                        octa value = current->opds[1]->value.num;
                        if (current->opds[1]->type == NEG_NUMBER)
                        {
                            operator++;
                            value = -value;
                        } else {
                            Instruction *curr_t;
                            int cnt = 0;
                            int finval = 0;
                            for (curr_t = current->next; curr_t && cnt < value - 1; cnt++, curr_t = curr_t->next)
                            {
                                if (curr_t->op->opcode == CALL)
                                    finval += 4;
                                else if (curr_t->op->opcode == PUSH)
                                    finval += 2;
                                else if (curr_t->op->opcode == RET)
                                    finval += 3;
                                else
                                    finval++;
                            }
                        }
                        machine_code = create_instr_d((operator << 24) +
                                (current->opds[0]->value.reg << 16) + (0xffff & value));
                    } else
                        machine_code = create_instr_i(current->opds[1]->value.label,
                                operator,
                                current->opds[0]->value.reg);
                    append_instr(&compiled, machine_code);
                    curr_instr ++;
                    break;
                }
            default:
                {
                    Instr *machine_code;
                    unsigned char operator = current->op->opcode;
                    if (current->op->opd_types[2])
                    {
                        unsigned char byte1, byte2, byte3;
                        if (current->opds[0]->type & REGISTER)
                            byte1 = current->opds[0]->value.reg;
                        else
                            byte1 = current->opds[0]->value.num;

                        if (current->opds[1]->type & REGISTER)
                            byte2 = current->opds[1]->value.reg;
                        else
                            byte2 = current->opds[1]->value.num;

                        if (current->opds[2]->type & REGISTER)
                            byte3 = current->opds[2]->value.reg;
                        else
                        {
                            byte3 = current->opds[2]->value.num;
                            operator++;
                        }

                        machine_code = create_instr_d((operator << 24) |
                                (byte1 << 16) | (byte2 << 8) | byte3);
                    } else if (current->op->opd_types[1])
                    {
                        unsigned char byte1;
                        unsigned short word1;
                        if (current->opds[0]->type & REGISTER)
                            byte1 = current->opds[0]->value.reg;
                        else
                            byte1 = current->opds[0]->value.num;

                        word1 = current->opds[1]->value.num;

                        machine_code = create_instr_d((operator << 24) |
                                (byte1 << 16) | word1);
                    } else if (current->op->opd_types[0])
                    {
                        unsigned int three1;
                        three1 = current->opds[0]->value.num;

                        machine_code = create_instr_d((operator << 24) |
                                (three1 & 0xffffff));
                    } else {
                        machine_code = create_instr_d(operator << 24);
                    }

                    append_instr(&compiled, machine_code);
                    curr_instr ++;
                    break;
                }
        }
    }

    int instr_cnt = 0;
    for (Instr *pointer = compiled; pointer; pointer = pointer->next) instr_cnt++;
    fprintf(outfile, "%d\n", instr_cnt);

    for (current = init; current && !errptr; current = current->next)
    {
        if (current->op->opcode == EXTERN)
        {
            EntryData *entry;
            if ((entry = stable_find(label_table, current->opds[0]->value.label)))
                fprintf(outfile, "E %s %d\n", current->opds[0]->value.label, entry->i);
            else
                die("Extern %s not found!!\n", current->opds[0]->value.label);
        }
    }

    fprintf(outfile, "B\n");

    curr_instr = 0;
    for (Instr *pointer = compiled; pointer; pointer = pointer->next)
    {
        if (pointer->opcode)
        {
            EntryData *entry;
            if (!(entry = stable_find(label_table, pointer->data.alias)))
                fprintf(outfile, "* %d %s\n", pointer->opcode, pointer->data.alias);
            else {
                unsigned char operator = pointer->opcode;
                int delta_l = entry->i - curr_instr;
                if (delta_l < 0)
                {
                    operator++;
                    delta_l = -delta_l;
                }
                unsigned int code = ((0xff & operator) << 24) + ((0xff & pointer->reg) << 16) + (0xffffff & delta_l);
                fprintf(outfile, "%08x\n", code);
            }
        } else {
            fprintf(outfile, "%08x\n", pointer->data.code);
        }
        curr_instr++;
    }

    // Destroy all
    instructions_destroy(&init);
    buffer_destroy(line);
    stable_visit(alias_table, del_item);
    stable_visit(label_table, del_item);
    stable_destroy(label_table);
    stable_destroy(alias_table);
    fclose(file);
}
