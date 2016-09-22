#include "stable.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char bool;

// Symbol table definition - Implemented as ternary search tree
struct stable_s {
    bool last_node;
    char value;
    EntryData data;
    SymbolTable lower, middle, higher;
};

// Return a new symbol table (Can be used for creating new symbol table nodes)
SymbolTable stable_create()
{
    SymbolTable table = (SymbolTable) malloc(sizeof(struct stable_s));
    table->last_node = 0;
    table->value = 0;
    table->lower = NULL;
    table->middle = NULL;
    table->higher = NULL;
    return table;
}

// Destroy a given symbol table.
void stable_destroy(SymbolTable table)
{
    stable_destroy(table->lower);
    stable_destroy(table->middle);
    stable_destroy(table->higher);
    free(table);
}

// Insert a new entry on the symbol table given its key.
InsertionResult stable_insert(SymbolTable table, const char *key)
{
    char *keychar = (char*) key;

    SymbolTable currnode = table;

    while(*keychar)
    {
        // If node not initialized
        if(!currnode->value) currnode->value = *keychar;

        // If current character is smaller than current node's
        if(*keychar < currnode->value)
        {
            // Create lower child case non-existent
            if(!currnode->lower) currnode->lower = stable_create();
            if(!currnode->lower)
            {
                printf("Failed allocating new node.\n");
                exit(-1);
            }

            // Navigate to lower child
            currnode = currnode->lower;
            continue;
        }

        // If current character is equal to current node's
        if(*keychar == currnode->value)
        {
            // Create middle child case non-existent
            if(!currnode->middle) currnode->middle = stable_create();
            if(!currnode->middle)
            {
                printf("Failed allocating new node.\n");
                exit(-1);
            }

            // Navigate to middle child and go to next character
            currnode = currnode->middle;
            keychar++;
            continue;
        }

        // If current character is greater than current node's
        if(*keychar > currnode->value)
        {
            // Create higher child case non-existent
            if(!currnode->higher) currnode->higher = stable_create();
            if(!currnode->higher)
            {
                printf("Failed allocating new node.\n");
                exit(-1);
            }

            // Navigate to higher child
            currnode = currnode->higher;
            continue;
        }
    }

    // If last node not initialized
    if(!currnode->value) currnode->value = *(keychar - 1);
    InsertionResult result;

    if(currnode->last_node) result.new = 0;
    else result.new = 1;

    currnode->last_node = 1;

    result.data = &currnode->data;

    return result;
}

// Find data associated with a given key.
EntryData *stable_find(SymbolTable table, const char *key)
{
    char *keychar = (char*) key;

    SymbolTable currnode = table;

    while(*keychar)
    {
        // If node not initialized
        if(!currnode->value) return NULL;

        // If current character is smaller than current node's
        if(*keychar < currnode->value)
        {
            // Key does not exist if node non-existent
            if(!currnode->lower) return NULL;
            // Navigate to lower child
            currnode = currnode->lower;
        }

        // If current character is equal to current node's
        if(*keychar == currnode->value)
        {
            // Key does not exist if node non-existent
            if(!currnode->middle) return NULL;

            // Navigate to middle child and go to next character
            currnode = currnode->middle;
            keychar++;
        }

        // If current character is greater than current node's
        if(*keychar > currnode->value)
        {
            // Key does not exist if node non-existent
            if(!currnode->higher) return NULL;

            // Navigate to higher child
            currnode = currnode->higher;
        }
    }

    // If last node not initialized
    if(!currnode->value) return NULL;

    if(!currnode->last_node) return NULL;
    else return &currnode->data;
}

// Side recursive function for iterating table entries
int stable_visit_rec(SymbolTable table, char *currstr, int *maxlen, int depth,
        int (*visit)(const char *key, EntryData *data))
{
    if(!table) return 1;

    if(depth >= *maxlen - 1)
    {
        *maxlen += 20;
        currstr = (char*) realloc(currstr, *maxlen + 1);
        if(!currstr)
        {
            printf("stable: stable_visit_rec: Failed to reallocate string.\n");
            exit(-1);
        }
    }

    currstr[depth] = table->value;

    if(table->last_node)
    {
        currstr[depth] = 0;
        if(!visit(currstr, &table->data)) return 0;
    }

    if(!stable_visit_rec(table->lower, currstr,
                maxlen, depth, visit)) return 0;

    if(!stable_visit_rec(table->middle, currstr,
                maxlen, depth + 1, visit)) return 0;

    if(!stable_visit_rec(table->higher, currstr,
                maxlen, depth, visit)) return 0;

    return 1;
}

// Visit each entry on the table
int stable_visit(SymbolTable table,
        int (*visit)(const char *key, EntryData *data))
{
    int maxlen = 20;
    char *string = (char*) malloc(maxlen);

    if(!string)
    {
        printf("stable: stable_visit: Failed to allocate string.\n");
        exit(-1);
    }

    int ret = stable_visit_rec(table, string, &maxlen, 0, visit);

    free(string);

    return ret;
}
