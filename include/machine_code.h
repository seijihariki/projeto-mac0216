#ifndef MACHINE_CODE_H
#define MACHINE_CODE_N
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
    unsigned char reg;
    unsigned char opcode;
    Instr* next;
};

/* Appends a given instruction to a given linked list of instructions.      *
 *                                                                          *
 * Params:                                                                  *
 * instr_chain: Chain (linked list) of instructions to append to            *
 * instruction: Instruction to append to linked list                        */
void append_instr (Instr **instr_chain, Instr *instruction);

/* Frees all objects of a given linked list of instructions, along with     *
 * strings of undefined instructions.                                       *
 *                                                                          *
 * Params:                                                                  *
 * instr_chain: Chain (linked list) of instructions to free                 */
void free_instr_chain (Instr **instr_chain);

/* Creates a undefined instruction from a given alias and opcode read from  *
 * the object file.                                                         *
 *                                                                          *
 * Params:                                                                  *
 * alias: Alias read from object file                                       *
 * opcode: Opcode read from object file                                     *
 *                                                                          *
 * Returns pointer to new instruction. 0 if an error occurred.              */
Instr* create_instr_i (char* alias, unsigned char opcode, unsigned char reg);

/* Creates a defined instruction from a given hexadecimal value read from   *
 * the object file.                                                         *
 *                                                                          *
 * Params:                                                                  *
 * code: The instruction, read as an hexadecimal value from object file     *
 *                                                                          *
 * Returns pointer to new instruction. 0 if an error occurred.              */
Instr* create_instr_d (unsigned int code);

#endif
