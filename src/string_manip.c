#include "string_manip.h"
#include "error.h"

#include <stdlib.h>
#include <ctype.h>

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
    for (; *w && *w != 10 && ((!is_str && *w != '*' && !isspace(*w) && *w != ',' && *w != ';') || (is_str && (*w != '"' || *(w - 1) == '\\'))); w++)
        sz++;

    if (is_str)
        sz+=2;

    return sz;
}
/* Returns the pointer to the start of the next word, checking for expected *
 * characters and ignoring spaces.                                          *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the search                                    *
 * expect: If this char doesn't appear until the start of the next word, it *
 * sets *error as the start of the next word and returns 0.                 *
 *                                                                          *
 * Returns:                                                                 *
 * Pointer to the start of the next word, zero if error or if EOL.          */
const char *nextWord(const char *w, char expect, const char **error)
{
    const char *ptr = w;
    byte found = 0;
    for (; *ptr && *ptr != 10 && *ptr != '*' && (isspace(*ptr) || (expect && *ptr == expect));
            ptr++)
    {
        if (*ptr == expect)
            found = 1;
    }

    if (!found && expect)
    {
        if (error)
            *error = (char *)ptr;
        return 0;
    }

    if (!*ptr)
    {
        if (error)
            *error = 0;
        ptr = 0;
    }
    return ptr;
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
