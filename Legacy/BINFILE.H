/****************************************************************
Copyright 1998 the author
Module Name    : BINFILE
Module Purpose : creation and use of binary files, allows data encoding
Creation Date  : 5 April 1998
Updated        : 1 May 1998         BinFileGetTill      CREATED
****************************************************************/
#ifndef BINFILE_H
#define BINFILE_H

#include <stdlib.h>
#include <stdio.h>

#ifndef byte
#define byte unsigned char
#endif

struct binfileTag
{
    int size;       // how many bytes are allocated
    int bytes;      // how many bytes are in use
    byte *store;    // storage of the bytes
};

typedef struct binfileTag BINFILE;

BINFILE *BinFileNew(int bytes);
// purpose : creates a new bin file of size bytes, bytes should be bigger
//           than required, as binfile size is static.
// returns : the binfile
BINFILE *BinFileRead(FILE *file,int size,int extras);
// purpose : reads a binfile of size bytes from a file, mallocing
//         : extras bytes more to the binfile
// returns : the binfile
BINFILE *BinFileReadFull(char *filename,int extras);
// purpose : assumes file filename is a binfile, reads it
// returns : the BINFILE, else NULL if does not exist
int     BinFileWrite(BINFILE *bf,FILE *file);
// purpose : write a binfile to a file
// returns : TRUE if successful else FALSE
int BinFileWriteFull(BINFILE *bf,char *filename);
// purpose : writes the binfile to file filename
// returns : TRUE if successful else FALSE
int BinFileRemain(BINFILE *bf);
// purpose : how many bytes are left to be put to or got from the binfile
// returns : the integer
int     BinFilePut(BINFILE *bf,int bytes,char *element);
// purpose : puts bytes from element in the binfile
// returns : TRUE if successful else FALSE
int BinFilePutBinfile(BINFILE *target,BINFILE *source);
// purpose : adds source binfile to the end of target binfile
// returns : TRUE is successful, FALSE otherwise
int     BinFileGet(BINFILE *bf,int bytes,char *element);
// purpose : gets up to bytes from the binfile, if cant get bytes
//           it gets as many as it can
// returns : the number of bytes gotten
int     BinFileGetTill(BINFILE *bf,int bytes,char *element,char till);
// purpose : gets bytes from the binfile , stops after reading till character
//           or bytes bytes are read.
// returns : 0 if no bytes read, else the number of bytes read
int     BinFilePutInt(BINFILE *bf,int element);
// purpose : puts the element in the binfile
// returns : TRUE if successful else FALSE
int     BinFileGetInt(BINFILE *bf);
// purpose : gets an int from the binfile
// returns : the integer
void    BinFilePutDouble(BINFILE *bf,double element);
// purpose : puts a double in the binfile
// returns : TRUE if successful else false
double  BinFileGetDouble(BINFILE *bf);
// purpose : gets a double in the binfile
// returns : TRUE if successful else false
void BinFilePrint(BINFILE *bf);
// purpose : prints the contents of the binfile to the screen
// returns : void
void BinFileFree(BINFILE *bf);
// purpose : frees the bin file from memory
// returns : void

// encryption algorithms follow, reverse of each is -ve parameter
// e.g. encrypt is BinFileEncryptAdd(BINFILE *bf,12)
//      decrypt is BinFileEncryptAdd(BINFILE *bf,-12)
// could use lots of them then decode in reverse order to make cracking
// difficult

void BinFileEncryptAdd(BINFILE *bf,byte a);
// adds a to every byte in the binfile, carry is lost.

void BinFileEncryptCarryAdd(BINFILE *bf,byte a);
// adds a to every byte in the binfile, carries remainder of byte add to
// the next byte add, when carry == 0, starts process again at current
// unencrypted byte

void BinFileEncryptShift(BINFILE *bf,int bytes);
// shifts the entire binfile by bytes, +ve is right, -ve is left;

void BinFileEncryptOddEven(BINFILE *bf);
// odd and even bits are swapped

void BinFileEncryptString(BINFILE *bf,char *string,int direction);
// adds the string byte by byte to each string lengthed section of the
// binfile. subtracts if direction <0 else adds

void BinFileEncryptInvert(BINFILE *bf);
// ones become zeros and zeros become ones.

void BinFileEncryptExpand(BINFILE *bf,int direction);
// if direction>0 a random bit is inserted after each bit 1111 -> 10101110
// else every second bit is removed 10101110 -> 1111 

#endif
