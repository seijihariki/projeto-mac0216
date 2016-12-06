#include "buffer.h"
#include "stable.h"
#include "string_manip.h"
#include "error.h"

#include <ctype.h>
#include <stdio.h>

int loadfile(FILE *file, SymbolTable externs, int start_instr)
{
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
                {
                    // ERROR!! WRONG FORMAT
                }

                InsertionResult inserted = stable_insert(externs, alias);

                free(alias);
                if (!inserted.new)
                {
                    // ERROR!! SAME NAME EXTERNS
                } else inserted.data->i = address;
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
                {
                    // ERROR!! WRONG FORMAT
                }

                
                break;
            }
            default:
                // ERROR!! WRONG FORMAT
                break;
            }
        } else {

        }
    }

    buffer_destroy(line);
    return start_instr + instr_cnt;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf ("Usage: %s <outfile> <file1> [file2 ... filen]\n", argv[0]);
        return 0;
    }


}
