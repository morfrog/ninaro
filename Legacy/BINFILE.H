#ifndef BINFILE_H
#define BINFILE_H

#include "sys.h"

struct binfileTag
{
    int size;       // how many bytes are allocated
    int atbyte;       // which byte is the current eof
    byte *store;    // storage of the bytes
};

typedef struct binfileTag BINFILE;

BINFILE *BinFileNew(int bytes);
// purpose : creates a new bin file of size bytes
// returns : the binfile
BINFILE *BinFileRead(char *filename);
// purpose : reads a bin file from a file
// returns : the binfile, else NULL if file does not exist
int     BinFileWrite(BINFILE *bf,char *filename);
// purpose : write a binfile to a file
// returns : TRUE if successful else false
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
// purpose : gets bytes from the binfile 
// returns : TRUE if successful else FALSE
int     BinFileGetTill(BINFILE *bf,int bytes,char *element,char till);
// purpose : gets bytes from the binfile , stops after reading till character
//           or bytes bytes are read.
// returns : TRUE if successful else FALSE
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
// note: some algorithms might like to append a number to front of binfile
//       which signifies the encryption amount. to do this move all stuff down
//       a byte and append at front or realloc, copy across and down 1 byte
void BinFileEncryptAdd(BINFILE *bf,int a);
// adds a to every byte in the binfile, carry is lost.

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
