#include "buffer.h"
#include "error.h"
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    int c, linec = 0, colNum, OK = 1, lastc = 0, firstc = 0, spc, i;
    FILE *input, *output;
    Buffer *line;
    line =  buffer_create();

    set_prog_name("Center");

    if (argc < 4) 
        die("Too few arguments supplied, expected 3.\n");

    else if (argc > 4) 
        die("Too many arguments supplied, expected 3.\n");
    sscanf(argv[3], "%d", &c);
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w+");
    line = buffer_create();

    while(OK)
    {
        colNum = read_line(input, line);
        if (colNum > 0 && colNum <= c)
        {
            for (i = 0; isspace(line->data[i]) > 0 && i < colNum ; i++);
            firstc = i;
            for (i += 1; i < colNum - 1; i++)
            {
                if(isspace(line->data[i]) > 0  && isspace(line->data[i + 1]) > 0)
                {
                    lastc = i;
                    break;
                }
            }
            if (lastc == 0)
                lastc = i;
            spc = (c - (lastc - firstc))/2;
            for (i = spc; i > 0; i--)
                fprintf(output, " ");
            for (i = firstc; i <= lastc; i--)
                fprintf(output, "%c", line->data[i]);
            for (i = spc; i > 0; i--)
                fprintf(output, " ");
            fprintf(output, "\n");
            

        }
        else
            OK = 0;
    }

    if (colNum > c)
        die(" %s: line %d: line too long.\n", argv[1], linec);
    
}
