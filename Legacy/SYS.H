#ifndef SYS_H
#define SYS_H
#include <stdlib.h>
enum {FALSE=0,TRUE};

#define byte unsigned char

// prefixes : S=SHIFT C=CTRL A=ALT KP=keypad(numlock off)
enum {BKSP=8,SENTER=10,ENTER=13,ESC=27,SPACE=32,DEL=127}; // Normal Keys
enum {Cat=0/* =C@ */,CA,CB,CC,CD,CE,CF,CG,CH,CI,CJ,CK,CL,CM,CN,CO,CP,CQ,CR,
      CS,CT,CU,CV,CW,CX,CY,CZ,
      F1=256+59,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11=256+133,F12,// Function Keys
      KP7=256+71,KP8,KP9,KP4=256+75,KP5,KP6,KP1=256+79,KP2,KP3,KP0,KPDEL,
                                                            // Keypad Keys
      SF1=256+84,SF2,SF3,SF4,SF5,SF6,SF7,SF8,SF9,SF10,      // Shift F Keys
      CF1,CF2,CF3,CF4,CF5,CF6,CF7,CF8,CF9,CF10,             // Ctrl F Keys
      AF1,AF2,AF3,AF4,AF5,AF6,AF7,AF8,AF9,AF10};            // Alt F Keys

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
int GetKey();
void IntToByteString(int in,char *array);
int ByteStringToInt(char *array);
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
