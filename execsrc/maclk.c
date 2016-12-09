#include "buffer.h"
#include "stable.h"
#include "string_manip.h"
#include "error.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

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
Instr* create_instr_i (char* alias, unsigned char opcode)
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
    ret->opcode = 0;
    ret->next = 0;
    return ret;
}

/* Loads symbols (externs) from object file, and parses it, transforming    *
 * local file references to global code line reference.                     *
 *                                                                          *
 * Params:                                                                  *
 * file: Object file to load symbols and code from                          *
 * externs: Table containing externs and the code line they refer to        *
 * start_instr: Line of code this file starts at, in a global context       *
 * instr_chain: Chain of instructions to add code of this file to           *
 *                                                                          *
 * Returns the number of the line of the code, in a global context, for the *
 * next instruction to be inserted (example: if the file has a start_instr  *
 * of 5, and there are 10 instructions in the file, the next instruction,   *
 * after this file, would be 15, and 15 would be returned).                 */
int loadfile(FILE *file, SymbolTable externs, int start_instr, Instr **instr_chain)
{
    if (!file)
        return -1;
    Buffer *line = buffer_create();
    int instr_cnt = 0;

    read_line(file, line);
    if (sscanf(line->data, "%d", &instr_cnt) < 1)
        return -1;

    while (read_line(file, line))
    {
        if (isspace(line->data[1]))
        {
            switch (line->data[0])
            {
            case 'E':
            {
                char *alias = malloc((getWord((const char*)&line->data[2]) + 1)*sizeof(char));
                int address;

                if (sscanf(line->data, "E %s %d", alias, &address) < 2)
                    die ("Line\n'%s'\ncould not be understood!", line->data);

                InsertionResult inserted = stable_insert(externs, alias);

                if (!inserted.new)
                    die("Label '%s' doubly defined!", alias);
                else inserted.data->i = address + start_instr;
                free(alias);
                break;
            }
            case 'B':
                break;
            case '*':
            {
                int i = 0, spacecnt = 0;
                while (spacecnt < 2)
                    spacecnt += isspace(line->data[i++]);
                char *alias = malloc((getWord((const char*)&line->data[i]) + 1)*sizeof(char));
                int opcode;

                if (sscanf(line->data, "* %d %s", &opcode, alias) < 2)
                    die ("Line\n'%s'\ncould not be understood!", line->data);

                Instr *instruction = create_instr_i(alias, opcode);
                append_instr(instr_chain, instruction);
                free(alias);
                break;
            }
            default:
                die ("Line\n'%s'\ncould not be understood!", line->data);
                break;
            }
        } else {
            unsigned int d_instruction;
            if (sscanf(line->data, "%x", &d_instruction) < 1)
                die ("Line\n'%s'\ncould not be understood!", line->data);
            Instr *instruction = create_instr_d(d_instruction);
            append_instr(instr_chain, instruction);
        }
    }

    buffer_destroy(line);
    return start_instr + instr_cnt;
}

/* This is the main function, which handles the inputs, load the files by   *
 * using loadfile, converts undefined to defined instructions and finally   *
 * writes resulting machine code into file. For ease of writing, the first  *
 * instruction is simply a JMP to the main label.                           */
int main(int argc, char **argv)
{
    set_prog_name(argv[0]);
    if (argc < 3)
    {
        printf ("Usage: %s <outfile> <file1> [file2 ... filen]\n", argv[0]);
        return 0;
    }

    FILE *outfile = fopen(argv[1], "wo");

    if (!outfile)
        die("Error opening file: '%s'\n", argv[1]);

    SymbolTable externs = stable_create();
    Instr* instr_chain = 0;

    // Instruction 0 is JMP to main, so first file starts at instruction 1
    int curr_instr = 1;

    for (int i = 2; i < argc; i++)
    {
        FILE *file = fopen(argv[i], "ro");
        if (!file) die("Error opening file: '%s'\n", argv[i]);
        curr_instr = loadfile(file, externs, curr_instr, &instr_chain);
        fclose(file);
    }

    EntryData *entry;

    if (!(entry = stable_find(externs, "main")))
    {
        die("Main was not defined!");
    } else {
        fprintf(outfile, "%x\n", (0x48 << 24) + entry->i);
    }

    if (!instr_chain)
    {
        // NOTHING TO DO
    }
    else
    {
        Instr *curr = instr_chain;
        curr_instr = 1;
        while (curr)
        {
            if (curr->opcode)
            {
                unsigned int instruction = 0;
                unsigned char opcode = curr->opcode;
                entry = stable_find(externs, curr->data.alias);

                if (!entry)
                    die ("Alias %s doesn't exist\n", curr->data.alias);

                int jmpdist = entry->i - curr_instr;
                if (jmpdist < 0)
                {
                    jmpdist = -jmpdist;
                    opcode++;
                }
                instruction = (opcode << 24) + jmpdist;
                free(curr->data.alias);
                curr->data.code = instruction;
                curr->opcode = 0;
            }
            curr_instr++;
            curr = curr->next;
        }

        curr = instr_chain;
        while (curr)
        {
            fprintf(outfile, "%08x\n", curr->data.code);
            curr = curr->next;
        }
    }

    free_instr_chain(&instr_chain);
    stable_destroy(externs);
}
