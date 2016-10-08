#include "buffer.h"
#include "error.h"
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    int c, linec = 0, charNum, OK = 1, lastc = 0, firstc = 0, spc, i;
    FILE *input, *output;
    Buffer *line;
    line =  buffer_create();

    set_prog_name("center");

    if (argc < 4) 
        die("Too few arguments supplied, expected 3.\n");

    else if (argc > 4) 
        die("Too many arguments supplied, expected 3.\n");

    c = atoi(argv[3]);

    input = fopen(argv[1], "r");
    if (input == 0)
        die("Error loading input file '%s'.\n", argv[1]);
    output = fopen(argv[2], "w+");
    if (output == 0)
        die("Error opening output file '%s'.\n", argv[2]);

    line = buffer_create();

    while(OK)
    {
        charNum = read_line(input, line);
        if (charNum > 0 && charNum <= c)
        {
            for (i = 0; isspace(line->data[i]) > 0 && i < charNum ; i++);
            firstc = i;
            for (i += 1; i < charNum - 1; i++)
            {
                if (!isspace(line->data[i]))
                    lastc = i;
            }

                printf("lalaallalalalla %d \n", i);
            spc = ((c - (lastc - firstc))/2) + 1;
            for (i = spc; i > 0; i--)
                fprintf(output, " ");
            for (i = firstc; i <= lastc; i++)
                fprintf(output, "%c", line->data[i]);
            fprintf(output, "\n");
        }
        else
            OK = 0;
    }

    if (charNum > c)
        die(" %s: line %d: line too long.\n", argv[1], linec);
    
}
