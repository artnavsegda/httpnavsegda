#include <stdio.h>
#include <stdlib.h>
#include "drop.h"

void drop(int dropstatus, char *dropdesc)
{
        if (dropstatus == -1)
        {
                perror(dropdesc);
                exit(1);
        }
}

void drop2(char *dropstatus, char *dropdesc)
{
        if (dropstatus == NULL)
        {
                perror(dropdesc);
                exit(1);
        }
}
