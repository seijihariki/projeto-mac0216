#include "stable.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int maxlen = 0;

int print_word(const char *key, EntryData *data)
{
    if(!key || !data) return 0;
	printf("%s:", key);
    int length = strlen(key);
    for(int i = length; i <= maxlen; i++) printf(" ");
    printf("%d\n", data->i);
	return 1;
}

// Returns length of longest word from current position on file
// Example:
//     FILE * fp = fopen("bar.txt", "r");
//     int len = longestWord(fp);
int longestWord(FILE * file)
{
    fpos_t pos;
    if(fgetpos(file, &pos)) return -1;
    char c;
    int sz, max;
    sz = max = 0;
    while((c = fgetc(file)) != EOF)
    {
        if(!isspace(c)) sz++;
        else
        {
            if(sz > max) max = sz;
            sz = 0;
        }
    }
    if(sz > max) max = sz;

    if(fsetpos(file, &pos)) return -1;
    return max;
}

// Main function
int main(int argc, char *argv[])
{
    set_prog_name("freq");
    set_error_msg("Failed to allocate memory.");
    if(argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(0);
    }

	FILE* fp;
	char *word;
	SymbolTable table = stable_create();

	fp = fopen(argv[1], "r");

	if(!fp) die("Could not open file %s.", argv[1]);

    maxlen = longestWord(fp);
    if (maxlen < 0) die("Couldn't determine longest word in file.");

    // Allocate enough for the longest word in the file
    word = (char*) emalloc((maxlen + 1)*sizeof(char));

	for (;fscanf(fp, "%s", word) != EOF;)
        stable_insert(table, word).data->i++;

	stable_visit(table, print_word);

	if(fclose(fp)) die("Could not close file %s. Exiting anyway.", argv[1]);

    return 0;
}
