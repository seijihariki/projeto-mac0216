#include "asm.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Modifies the file extension to .maco for the generated file.                 *
 *                                                                              *
 *    Params:                                                                   *
 *                                                                              *
 *     input_file: Name of the input file.                                      *
 *                                                                              */
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        exit(-1);
    }

    FILE *file = fopen(argv[1], "ro");
    char* objfile = objfile_name(argv[1]);
    FILE *outfile = fopen(objfile, "wo");

    if(!assemble(argv[1], file, outfile)) die(0);

    free(objfile);

    fclose(file);
    fclose(outfile);
}
