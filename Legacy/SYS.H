#ifndef SYS_H
#define SYS_H
#include <stdlib.h>
#include "defines.h"
#define byte unsigned char

struct iarray
{
    int size;
    int *array;
};

typedef struct iarray INTARRAY;

void *salloc(int n_items,size_t something);
char *allocate_array(int nelements, int sizeof_element, char *array_name);
char *allocate_N_by_N_array(int n1, int n2, int sizeof_element, char *array_name);
int sfree(void *ptr);
char mygetch();
void IntToByteString(int in,char *array);
int ByteStringToInt(char *array);
void _Break(int args,...);
long FileSize(char *directory,char *filename);
char *NewString(char *string);
void StartRandom();
int Random(int num);
void PrintList(int *list,int listsize);
int Same(int A,int B,int C, int D);
int PosOf(int num,int *list,int listsize);
int Lesser(double this,double best);
int Greater(double this,double best);
int IntLesser(int first,int second);
int IntGreater(int first,int second);
INTARRAY *NewIntArray(int size,int val);
void CopyIntArray(INTARRAY *to,INTARRAY *from);
void FreeIntArray(INTARRAY **a);
void Beep(int pitch,int time);
void Working();
void ClearBuffer();
int PlusFactorial(int num);

#endif
