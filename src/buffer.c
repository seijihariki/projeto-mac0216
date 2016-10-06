#include "buffer.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>

Buffer *buffer_create()
{
    Buffer *B;
    B = emalloc(sizeof(Buffer));
    B->n = 50;
    B->data = emalloc(B->n * sizeof(char));
    B->i = 0;
    return B;
} 

void buffer_destroy(Buffer *B)
{
    free(B->data);
    free(B);
}

void buffer_reset(Buffer *B)
{
	for (; B->i > 0; (B->i)--)
		B->data[B->i] = 0;
}

void buffer_push_back(Buffer *B, char c)
{
    //If Buffer is full
    if (B->i == B->n)
    {
        B->n *= 2;
        B->data = realloc(B->data, B->n * sizeof(char));
        if (B->data == NULL)
        {
            free(B->data);
            free(B);
            die("Out of memory: failed to reallocate Bufferi\n");
        }
    }

	B->data[B->i] = c;
    B->i++;
    
}

int read_line(FILE *input, Buffer *B)
{
    int c = 0, cnum = 0;
    buffer_reset(B);
    while(c != 10 && c != EOF)
    {
        c = fgetc(input);
        if (c != EOF)
        {
            buffer_push_back(B, (char)c);
            cnum++;
        }
    }
    return cnum;
}
