/* Sets ADT */

#include <stdio.h>
#include <stdlib.h>
#include "set.h"

SET *SetNew(int size)
{
    int x,bytes=(size/CHAR_BITS)+(size%CHAR_BITS?1:0);
    SET *set=malloc(sizeof(SET));
    set->size=size;
    set->set=malloc(bytes);
    for (x=0;x<bytes;x++)
        set->set[x]=0;
    return set;
}

void SetAdd(SET *set,int n)
{
    unsigned char bit=n%CHAR_BITS;
    unsigned char byte=n/CHAR_BITS;
    set->set[byte]=set->set[byte] | (1 << (CHAR_BITS-bit-1));
}


void SetDel(SET *set,int n)
{
    unsigned char bit=n%CHAR_BITS;
    unsigned char byte=n/CHAR_BITS;
    set->set[byte]=~(~set->set[byte] | (1 << (CHAR_BITS-bit-1)));
}

int SetMember(SET *set,int n)
{
    unsigned char bit=n%CHAR_BITS;
    unsigned char byte=n/CHAR_BITS;
    return (set->set[byte] & (1 << (CHAR_BITS-bit-1)));
}

void SetPrint(SET *set)
{
    int x;
    for (x=0;x<set->size;x++)
        if (SetMember(set,x)) printf("1"); else printf("0");
    printf("\n");
}

void SetFree(SET *set)
{
    free(set->set);
    free(set);
}

void main()
{
    SET *set=SetNew(9);
    SetAdd(set,0);
    SetAdd(set,8);
    SetAdd(set,5);
    SetPrint(set);
    SetDel(set,5);
    SetPrint(set);
    SetFree(set);
}
