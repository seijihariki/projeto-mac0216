#include "buffer.h"
#include "stable.h"
#include "error.h"

#include <ctype.h>
#include <stdio.h>

typedef unsigned char byte;

/* Returns the size of a word starting at w (until space, newline, comma or *
 * semicolon). Exception - If w[0] is ", then read until an unescaped ".    *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the word                                      *
 *                                                                          *
 * Returns:                                                                 *
 * Size of word found at given pointer                                      */
int getWord(const char *w)
{
    int sz = 0;
    byte is_str = 0;
    if (w[0] == '"')
    {
        is_str = 1;
        w++;
    }
    for (; *w && *w != 10 && ((*w != '*' && !isspace(*w) && *w != ',' && *w != ';')
                || (is_str && (*w != '"' || *(w - 1) == '\\'))); w++)
        sz++;
    return sz;
}

/* Reads word starting at w to a separate string. If wl is given, its value *
 * is set to that of the length of the word that was read.                  *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the search                                    *
 * wl: Pointer to an integer to put the size of the word into. (Optional)   *
 *                                                                          *
 * Returns:                                                                 *
 * Pointer to the location of the newly allocated word.                     */
const char *readWord(const char *w, int *wl)
{
    if (!w)
        return 0;
    int word_len = getWord(w);

    if (wl)
        *wl = word_len;
    char *word = emalloc((word_len + 1) * sizeof(char));
    for (int i = 0; i <= word_len; i++)
        word[i] = w[i];

    word[word_len] = 0;
    return word;
}

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

                if (sscanf(line->data, "E %s %d", &alias, &address) < 2)
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

                if (sscanf(line->data, "* %d %s", &opcode, &alias) < 2)
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
        printf ("Usage: %s <outfile> <file1> [file2 ... filen]\n");
        return 0;
    }


}
