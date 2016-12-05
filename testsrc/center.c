#include "buffer.h"
#include "error.h"
#include <ctype.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    //declare and initialize variables
    int c, linec = 0, charNum, OK = 1, lastc = 0, firstc = 0, spc, i;
    FILE *input, *output;
    Buffer *line;
    line = buffer_create();

    set_prog_name("center");
    //checks the numbere of arguments
    if (argc < 4)
        die("Too few arguments supplied, expected 3.\n");

    else if (argc > 4)
        die("Too many arguments supplied, expected 3.\n");
    //pick column number from arguments
    c = atoi(argv[3]);
    //receives the input and output files and checks if input file exists and
    //output file exists or can be created
    input = fopen(argv[1], "r");
    if (input == 0)
        die("Error loading input file '%s'.\n", argv[1]);
    output = fopen(argv[2], "w+");
    if (output == 0)
        die("Error opening output file '%s'.\n", argv[2]);

    line = buffer_create();

    while (OK)
    {
        linec++;
        charNum = read_line(input, line);
        //if the line has zero chars
        if (charNum == 0)
            OK = 0;

        else
        { //find the first non-space char
            for (i = 0; isspace(line->data[i]) > 0 && i < charNum; i++)
                ;
            firstc = i;
            //find the last non-space char
            for (i += 1; i < charNum - 1; i++)
            {
                if (!isspace(line->data[i]))
                    lastc = i;
            }
            //if the line trimmed is bigger than specified
            if ((lastc - firstc) > c)
                break;
            //calculates numbere of spaces for centering
            spc = ((c - (lastc - firstc)) / 2) + 1;
            //prints everything
            for (i = spc; i > 0; i--)
                fprintf(output, " ");

            for (i = firstc; i <= lastc; i++)
                fprintf(output, "%c", line->data[i]);

            fprintf(output, "\n");
        }
    }
    //show message error for line too long
    if ((lastc - firstc) > c)
        die(" %s: line %d: line too long.\n", argv[1], linec);
}
