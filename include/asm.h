#ifndef ASM_H
#define ASM_H
#include <stdio.h>
#include "error.h"
/*Assembles and does everything                                                 *
 *                                                                              *
 *                                                                              *
 *                                                                              */
int assemble(const char *filename, FILE *input, FILE *output);
#endif
