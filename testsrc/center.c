#include "buffer.h"
#include "error.h"
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    int c, linec, colNum, OK = 1, lastc = 0, firstc = 0, spc, i;
    FILE *input, *output;
    Buffer *line;

    if (argc < 3) 
        die("Too few arguments supplied, 3 espected.\n");

    else if (argc > 3) 
        die("Too many arguments supplied, 3 expected.\n");

    set_prog_name(argv[0]);
    c = argv[3];  
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w+");
    line = buffer_create();

    while(OK)
    {
        colNum = read_line(input, line);
        if (colNum > 0 && colNum <= c)
        {
            for (i = 0; isspace(line[i]) && i < colNum ; i++);
            firstc = i;
            for (i += 1; i < colNum - 1; i++)
            {
                if(isspace(line[i]) && isspace(line[i + 1]))
                {
                    lastc = i;
                    break;
                }
            }
            if (lastc == 0)
                lastc = i;
            spc = (c - (lastc - firstc))/2;
            for (i = spc; i > 0; i--)
                fprintf(" ");
            for (i = firstc; i <= lastc; i--)
                fprintf("%c", line[i]);
            for (i = spc; i > 0; i--)
                fprintf(" ");
            fprintf("\n");
            

        }
        else
            ok = 0;
    }

    if (colNum > c)
        die(" %s: line %d: line too long.\n", argv[1], linec);
    
}
