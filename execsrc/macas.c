#include "buffer.h"
#include "error.h"
#include "parser.h"
#include "stable.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int del_item(const char *key, EntryData *data)
{
    if (data->opd)
        free(data->opd);
    return 1;
}

int main(int argc, char *argv[])
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
        //isso aqui cria uma linked list com as instruçoes e elas tem lineno
        //ja :)
        if (!parse(line->data, alias_table, &init, &errptr))
        {
            printf("%s", line->data);
            for (int i = 0; i < (unsigned int)((errptr - line->data) / sizeof(char)); i++)
                printf((line->data[i] == '\t') ? "\t" : " ");
            printf("^\n");
            print_error_msg(0);
            break;
        }
    }

    for (current = init; current; current = current->next)
    {
        //fazer codigo de percorrer a lista ligada
    }

    //verificar se mantem esse erro
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
                    die("Extern cannot point to IS instruction\n");
            }
        }
        if (!ok)
            die("Extern label %s never defined\n", current->opds[0]->value.label);
    }
}

// Destroy all
instructions_destroy(&init);
buffer_destroy(line);
stable_visit(alias_table, del_item);
stable_destroy(alias_table);
fclose(file);
}
