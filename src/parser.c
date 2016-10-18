/*
  parser.c
*/

#include "error.h"
#include "parser.h"
#include <cstdio>

/* Returns size of command starting at w (until newline or semicolon)       *
 *                                                                          *
 * Params:                                                                  *
 * c: Pointer to the start of the command                                   *
 *                                                                          *
 * Returns:                                                                 *
 * Size of command fount aat given pointer                                  */
int getCommand(const char *c)
{
    int sz = 0;
    for(; *w && *w != '\n' && *w != ';'; w++) sz++;

}

/* Returns the size of a word starting at w (until space, newline, comma or *
 * semicolon).                                                              *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the word                                      *
 *                                                                          *
 * Returns:                                                                 *
 * Size of word found at given pointer                                      */
int getWord(const char *w)
{
    int sz = 0;
    for(; *w && !isspace(*w) && *w != ',' && *w != ';'; w++) sz++;
    return sz;
}

/* Returns the pointer to the start of the next word, ignoring spaces,      *
 * newlines, commas, and semicolons                                         *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the search                                    *
 *                                                                          *
 * Returns:                                                                 *
 * Pointer to the start of the next word                                    */
char* nextWord(const char *w)
{
    char* ptr = w;
    for(; *ptr && isspace(*ptr) && *ptr == ',' && *ptr == ';'; ptr++);
    if(!*ptr) ptr = 0;
    return ptr;
}

int parse(const char *s, SymbolTable alias_table, Instruction **instr,
          const char **errptr)
{
    
    return 1;
}
