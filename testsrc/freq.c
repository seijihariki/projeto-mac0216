#include "stable.h"

#include <stdio.h>
#include <stdlib.h>

int print_word(const char *key, EntryData *data)
{
	printf("%s: %d\n", key, data->i);
	return 1;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(0);
    }

	FILE* fp;
	char word[1024]; // Assuming no word is longer than 1024 characters
	SymbolTable table = stable_create();

	fp = fopen(argv[1], "r");

	if(!fp)
	{
		printf("Could not open file %s.\n", argv[1]);
		exit(-1);
	}

	for (;fscanf(fp, "%s", word) != EOF;) stable_insert(table, word).data->i++;

	stable_visit(table, print_word);

	if(fclose(fp))
	{
		printf("Could not close file %s. Exiting anyway.\n", argv[1]);
		exit(-1);
	}

    return 0;
}
