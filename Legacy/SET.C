/* Sets ADT */
#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "set.h"

void SetEmpty(SET *set)
// purpose : to empty the set, set all elements to false
// returns : void
{
    int x;
    int bytes=(set->size/CHAR_BITS)+(set->size%CHAR_BITS?1:0);
    for (x=0;x<bytes;x++)
        set->set[x]=0;
}

SET *SetNew(int size)
// purpose : to create a new set of size elements
// returns : the set
{
    int bytes=(size/CHAR_BITS)+(size%CHAR_BITS?1:0);
    SET *set=malloc(sizeof(SET));
    set->size=size;
    set->set=malloc(bytes*sizeof(char));
    SetEmpty(set);
    return set;
}

void SetAdd(SET *set,int n)
// purpose : set element n of the set to TRUE
// returns : void
{
    unsigned char bit=n%CHAR_BITS;
    unsigned char byte=n/CHAR_BITS;
    set->set[byte]=set->set[byte] | (1 << (CHAR_BITS-bit-1));
}


void SetDel(SET *set,int n)
// purpose : set element n of the set to FALSE
// returns : void
{
    unsigned char bit=n%CHAR_BITS;
    unsigned char byte=n/CHAR_BITS;
    set->set[byte]=~(~set->set[byte] | (1 << (CHAR_BITS-bit-1)));
}

int SetMember(SET *set,int n)
// purpose : to find if element n of the set is TRUE
// returns : TRUE if n is member else FALSE
{
    unsigned char bit=n%CHAR_BITS;
    unsigned char byte=n/CHAR_BITS;
    return (set->set[byte] & (1 << (CHAR_BITS-bit-1))?TRUE:FALSE);
}

void SetPrint(SET *set)
// purpose : to print the contents of the set
// returns : void
{
    int x;
    for (x=0;x<set->size;x++)
        if (SetMember(set,x)) printf("1"); else printf("0");
    printf("\n");
}

void SetFree(SET *set)
// purpose : to free memory allocated to the set
// returns : void
{
    free(set->set);
    free(set);
}

/*
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
*/
