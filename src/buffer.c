#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>

Buffer *buffer_create()
{
    Buffer *B;
    B->data  = malloc(B->n * sizeof(char));
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
    //Aumenta o vetor data caso não tenha espaco suficiente
    if (B->i == B->n)
    {
        char *aux;
        int j;
        aux = malloc((B->n * 2) * sizeof(char));
        for (j = 0; j < B->n; j++)
            aux[j] = B->data[j];
        B->n = B->n * 2;
        free(B->data);
        B->data = aux;
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
