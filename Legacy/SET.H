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
/* empty & create set */

void SetAdd(SET *set,int n);      
/* add n to set */

void SetDel(SET *set,int n);      
/* remove n from set */

int SetMember(SET *set,int n);      
/* test if n is in set */

void SetPrint(SET *set);
/* prints the set, using 1's and 0's */

void SetFree(SET *set);

#endif
