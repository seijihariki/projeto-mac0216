#include "parser.h"
#include "stable.h"
#include "buffer.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void instructions_destroy(Instruction **instr_list)
{
    Instruction *current = *instr_list, *last = 0;
    while (current)
    {
        last = current;
        current = current->next;
        instr_destroy(last);
    }
}

int del_item(const char* key, EntryData *data)
{
    if (data->opd)
        free(data->opd);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        exit(-1);
    }

    // Initialize all
    set_prog_name("parse_test");
    SymbolTable alias_table = stable_create();
    FILE *file = fopen(argv[1], "ro");
    Buffer *line = buffer_create();
    Instruction *current = 0;
    Instruction *init = 0;

    const char *errptr = 0;
    while (read_line(file, line))
    {
        if (!parse(line->data, alias_table, &init, &errptr))
            break;
        if (!init) init = current;
    }

    Instruction *ext_check = 0;
    for (current = init; current; current = current->next)
    {
        if (current->op->opcode == EXTERN)
        {
            int ok = 0;
            for (ext_check = current->next; ext_check; ext_check = ext_check->next)
            {
                if (ext_check->label && !strcmp(ext_check->label, current->opds[0]->value.label))
                {
                    if (ext_check->op->opcode != IS)
                        ok = 1;
                    else
                        die ("Extern cannot point to IS instruction\n");
                }
            }
            if (!ok)
                die ("Extern label %s never defined\n", current->opds[0]->value.label);
        }
    }


    for (current = init; current; current = current->next)
    {
        if(errptr)
        {
            printf("%s", line->data);
            for (int i = 0; i < (unsigned int)((errptr - line->data)/sizeof(char)); i++)
                printf((line->data[i] == '\t')?"\t":" ");
            printf("^\n");
            die(0);
        }
        printf("line     = %s\n", line->data);
        printf("label    = \"%s\"\n", current->label);
        printf("operator = %s\n", current->op->name);
        printf("operands = ");
        for (int i = 0; i < 3; i++)
        {
            if (current->opds[i])
            {
                if (i != 0)
                    printf(", ");
                switch (current->opds[i]->type)
                {
                case REGISTER:
                    printf("Register(%u)", current->opds[i]->value.reg);
                    break;
                case NUMBER_TYPE:
                    printf("Number(%lld)", current->opds[i]->value.num);
                    break;
                case LABEL:
                    printf("Label(\"%s\")", current->opds[i]->value.label);
                    break;
                case STRING:
                    printf("String(%s)", current->opds[i]->value.str);
                    break;
                }
            }
        }
        printf("\n\n");
    }

    // Destroy all
    instructions_destroy(&init);
    buffer_destroy(line);
    stable_visit(alias_table, del_item);
    stable_destroy(alias_table);
    fclose(file);
}
