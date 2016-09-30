#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>

Buffer *buffer_create()
{
    Buffer *B;
    B->data  = malloc(B->n * sizeof(char));
    if (B->data == NULL)
    {
        //Not enough memory to create the buffer
        printf("unable to create Buffer: Out of memory");
        free(B->data);
        exit(-1);
    }
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
        char *aux;
        int j;
        aux = malloc((B->n * 2) * sizeof(char));
        if (aux == NULL)
        {
            //Not enough memory to reallocate buffer
            printf("Unable to reallocate Buffer: Out of memory");
            buffer_destroy(B);
            exit(-1);
        }
        for (j = 0; j < B->n; j++)
            aux[j] = B->data[j];
        B->n = B->n * 2;
        free(B->data);
        B->data = aux;
        free(aux);
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
