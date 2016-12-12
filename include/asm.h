#ifndef ASM_H
#define ASM_H
#include <stdio.h>
#include "error.h"
/*Assembles and does everything needed to generate the .maco file.              *
 *                                                                              *
 *Params:                                                                       *
 * filename: the name for the output file.                                      *
 *input and output: input file and output file.                                 */
int assemble(const char *filename, FILE *input, FILE *output);
#endif
