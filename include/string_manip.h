#ifndef __STR_MAN_H__
#define __STR_MAN_H__

typedef unsigned char byte;

/* Returns the size of a word starting at w (until space, newline, comma or *
 * semicolon). Exception - If w[0] is ", then read until an unescaped ".    *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the word                                      *
 *                                                                          *
 * Returns:                                                                 *
 * Size of word found at given pointer                                      */
int getWord(const char *w);

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
const char *nextWord(const char *w, char expect, const char **error);

/* Reads word starting at w to a separate string. If wl is given, its value *
 * is set to that of the length of the word that was read.                  *
 *                                                                          *
 * Params:                                                                  *
 * w: Pointer to the start of the search                                    *
 * wl: Pointer to an integer to put the size of the word into. (Optional)   *
 *                                                                          *
 * Returns:                                                                 *
 * Pointer to the location of the newly allocated word.                     */
const char *readWord(const char *w, int *wl);



#endif
