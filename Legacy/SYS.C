// SYS_C
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <conio.h>
#include <time.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include "sys.h"

void *salloc(int n_items,size_t something) {
/**********************************************************************
Description   : Allocates memory, displays an error if there isn't enough
Pre-Condition : The amount of memory to allocate is passed in.
Post-Condition: A pointer to this memory is returned.
**********************************************************************/
    void *p=NULL;
    if ( (p=malloc(n_items*something)) == 0 )
    {
        printf("Out Of Memory Error.\n\n\nGoodBye.\n");
        exit(EXIT_FAILURE);
    }
    /*
    else
    {
        printf("%d Memory Allocated",_msize(p) / something);
        if ((_msize(p) / something)!=n_items) printf(":UNEXPECTED - Required %d.",n_items);
        printf("\n");
    }
    */   
    return p;
}

char *
allocate_array(int nelements, int sizeof_element, char *array_name)
{
    char    *p=NULL;
    int i;


    if ( (p=salloc(nelements,sizeof_element)) == 0 )
    {
        printf("allocate_array: error in salloc() for array '%s'\n",array_name);
        exit(1);
    }

    for ( i=0 ; i<nelements*sizeof_element ; ++i ) p[i] = 0;

    return(p);

} /* allocate_array */

char *
allocate_N_by_N_array(int n1, int n2, int sizeof_element, char *array_name)
{
    char    **p=NULL;
    int i;


    p = (char **) allocate_array(n1, sizeof(char *), array_name);

    for ( i=0; i<n1 ; ++i )
    {
        p[i] = allocate_array(n2, sizeof_element, array_name);
    }

    return( (char *) p);

} /* allocate_N_by_N_array */

int sfree(void *ptr)
{
    if (ptr==NULL)
        return 0;
    else
        free(ptr);
    return 1;
}

char mygetch()
{
    printf("Hit any key...\n");
    return getch();
}

void StartRandom()
{
    struct dostime_t time;
    _dos_gettime(&time);
    srand(((time.hsecond)*(time.second))%RAND_MAX);
}

int Random(int num)
{
    return (rand() % num); /* 0..num-1 */
}

void PrintList(int *list,int listsize)
{
    int x;
    for (x=0;x<listsize;x++)
        printf("%2d ",list[x]);
    printf("\n");
}

int Same(int A,int B,int C, int D)
{
    return ( ((A==D) && (B==C)) || ((A==C) && (B==D)) );

}

int PosOf(int num,int *list,int listsize)
/* returns pos of num in unsorted list, -1 of not in list */
{
    int x;
    for (x=0;x<listsize;x++)
        if (list[x]==num) return x;
    return -1;
}

int Lesser(double this,double best)
{
    if (this<best) return 1;
    return 0;
}

int Greater(double this,double best)
{
    if (this>best) return 1;
    return 0;
}

int IntLesser(int first,int second)
{
    if (first<second) return 1;
    return 0;
}

int IntGreater(int first,int second)
{
    if (first>second) return 1;
    return 0;
}

INTARRAY *NewIntArray(int size,int val)
{
    INTARRAY *ret=salloc(1,sizeof(INTARRAY));
    int x;
    ret->size=size;
    ret->array=salloc(size,sizeof(int));
    for (x=0;x<size;x++) ret->array[x]=val;
    return ret;
}

void CopyIntArray(INTARRAY *to,INTARRAY *from)
{
    int x;
    for (x=0;x<to->size;x++)
        to->array[x]=from->array[x];
}

void FreeIntArray(INTARRAY **a)
{
    sfree((*a)->array);
    sfree(*a);
}

void Beep(int pitch,int time)
{
    sound(pitch);
    delay(time);
    nosound();
}

void Working()
{
    static char array[4]={'|','/','-',92};
    static int pos=0;
    static char out[2]="  ";

    out[0]=array[pos];
    pos=(pos+1)%4;
    _settextposition(1,1);
    _outtext(out);
}

void ClearBuffer()
{
    while (kbhit()) getch();
}

int PlusFactorial(int num)
{
    int x,count=0;
    for (x=1;x<=num;x++)
        count+=x;
    return count;        
}

