/****************************************************************
Copyright 1998 the author
Module Name    : SET
Module Purpose : to provide a set ADT
Creation Date  : 3 April 1998
Updated        : None
****************************************************************/
/* Sets ADT */

#ifndef SET_H
#define SET_H

#define CHAR_BITS          8 /* number of bits in char */

struct setTag
{
    int size;
    unsigned char *set;
};

typedef struct setTag SET;

SET *SetNew(int size);
// purpose : to create a new set of size elements
// returns : the set
void SetEmpty(SET *s);
// purpose : to empty the set, set all elements to false
// returns : void
void SetAdd(SET *set,int n);
// purpose : set element n of the set to TRUE
// returns : void
void SetDel(SET *set,int n);
// purpose : set element n of the set to FALSE
// returns : void
int SetMember(SET *set,int n);
// purpose : to find if element n of the set is TRUE
// returns : TRUE if n is member else FALSE
void SetPrint(SET *set);
// purpose : to print the contents of the set
// returns : void
void SetFree(SET *set);
// purpose : to free memory allocated to the set
// returns : void

#endif
