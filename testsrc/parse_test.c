#include "parser.h"
#include "stable.h"
#include <stdio.h>
#include <stdlib.h>
int main()
{
    SymbolTable alias_table = stable_create();
    Instruction **instr = malloc(4 * sizeof(Instruction *));
    const char *errptr = 0;
    parse("a IS $27; teste DIV     $0,$0,#70;  MUL $2,    $5, #20; ADD $6, a, $5", alias_table, instr, &errptr);
    for (int l = 0; l < 4; l++)
    {
        if (!instr[l])
        {
        }
        printf("label    = \"%s\"\n", instr[l]->label);
        printf("operator = %s\n", instr[l]->op->name);
        printf("operands = ");
        for (int i = 0; i < 3; i++)
        {
            if (instr[l]->opds[i])
            {
                if (i != 0)
                    printf(", ");
                switch (instr[l]->opds[i]->type)
                {
                case REGISTER:
                    printf("Register(%u)", instr[l]->opds[i]->value.reg);
                    break;
                case NUMBER_TYPE:
                    printf("Number(%lld)", instr[l]->opds[i]->value.num);
                    break;
                case LABEL:
                    printf("Label(\"%s\")", instr[l]->opds[i]->value.label);
                    break;
                case STRING:
                    printf("String(%s)", instr[l]->opds[i]->value.str);
                    break;
                }
            }
        }
        printf("\n");
    }
}
