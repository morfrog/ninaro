#include <stdlib.h>
#include <stdio.h>
#include "binfile.h"

BINFILE *BinFileNew(int bytes)
// purpose : creates a new bin file of size bytes
// returns : the binfile
{
    int x;
    BINFILE *bf=malloc(sizeof(BINFILE));
    bf->size=bytes;
    bf->bytes=0;
    bf->store=malloc(bytes*sizeof(byte));
    for (x=0;x<bytes;x++) bf->store[x]=0;
    return bf;
}

BINFILE *BinFileRead(FILE *file,int size,int extras,int *read)
// purpose : reads a binfile of size bytes from a file, mallocing
//         : extras bytes more to the binfile
// returns : the binfile
{
    BINFILE *bf=BinFileNew(size+extras);
    fread(bf->store,sizeof(byte),(size_t)size,file);
    bf->bytes=0;
    return bf;
}

BINFILE *BinFileReadFull(char *filename,int extras)
// purpose : assumes file filename is a binfile, reads it
// returns : the BINFILE, else NULL if does not exist
{
    int read;
    FILE *file=fopen(filename,"rb");
    BINFILE *bf;
    if (file==NULL) return NULL;
    bf=BinFileRead(file,(int)FileSize(".",filename),extras,&read);
    fclose(file);
    return bf;
}

int     BinFileWrite(BINFILE *bf,FILE *file)
// purpose : write a binfile to a file
// returns : TRUE if successful else FALSE
{
    fwrite(bf->store,sizeof(byte),bf->bytes,file);
    return TRUE;
}

int BinFileWriteFull(BINFILE *bf,char *filename)
// purpose : writes the binfile to file filename
// returns : TRUE if successful else FALSE

{
    FILE *file=fopen(filename,"wb");
    if (file==NULL) return FALSE;
    BinFileWrite(bf,file);
    fclose(file);
    return TRUE;
}

int BinFileRemain(BINFILE *bf)
// purpose : how many bytes are left to be put to or got from the binfile
// returns : the integer
{
    return ((bf->size)-(bf->bytes));
}

int     BinFilePut(BINFILE *bf,int bytes,char *element)
// purpose : puts bytes from element into the binfile
// returns : TRUE if successful else FALSE
{
    int x;
    if (BinFileRemain(bf)<bytes) return FALSE;
    else
    {
        for (x=0;x<bytes;x++)
        {
            bf->store[bf->bytes]=element[x];
            bf->bytes+=1;
        }
        return TRUE;
    }
}

int BinFilePutBinfile(BINFILE *target,BINFILE *source)
// purpose : adds source binfile to the end of target binfile
// returns : TRUE is successful, FALSE otherwise
{
    if (BinFileRemain(target)<source->bytes)
        return FALSE;
    else
        return BinFilePut(target,source->bytes,source->store);
}

int     BinFileGet(BINFILE *bf,int bytes,char *element)
// purpose : get bytes from the binfile
// returns : TRUE if successful else FALSE
{
    int x;
    if (BinFileRemain(bf)<bytes) return FALSE;
    printf("Getting %d Bytes.",bytes);
    for (x=0;x<bytes;x++)
    {
        element[x]=bf->store[bf->bytes];
        bf->bytes+=1;
    }
    return TRUE;
}

int     BinFileGetTill(BINFILE *bf,int bytes,char *element,char till)
// purpose : gets bytes from the binfile , stops after reading till character
//           or bytes bytes are read.
// returns : TRUE if successful else FALSE
{
    int x;
    if (BinFileRemain(bf)<bytes) return FALSE;
    for (x=0;x<bytes;x++)
    {
        element[x]=bf->store[bf->bytes];
        bf->bytes+=1;
        if (element[x]==till) return TRUE;
    }
    return TRUE;
}

int     BinFilePutInt(BINFILE *bf,int element)
// purpose : puts the element in the binfile
// returns : TRUE if successful else FALSE
{

    char buffer[4];
    IntToByteString(element,buffer);
    return BinFilePut(bf,4,buffer);
}

int     BinFileGetInt(BINFILE *bf)
// purpose : gets an int from the binfile
// returns : the integer
{
    char buffer[4];
    if (BinFileGet(bf,4,buffer))
        return ByteStringToInt(buffer);
    else return -1; // not perfect, assumes -1 not a value which might be used
}

/*
void    BinFilePutDouble(BINFILE *bf,double element)
// purpose : puts a double in the binfile
// returns : TRUE if successful else false
{

}

double  BinFileGetDouble(BINFILE *bf)
// purpose : gets a double in the binfile
// returns : TRUE if successful else false
{

}

*/

void BinFilePrint(BINFILE *bf)
// purpose : prints the contents of the binfile to the screen
// returns : void
{
    int x;
    printf("Size = %d \nBytes = %d\n'",bf->size,bf->bytes);
    for (x=0;x<bf->size;x++)
        printf("%c",bf->store[x]);
    printf("'\n");
}

void BinFileFree(BINFILE *bf)
// purpose : frees the bin file from memory
// returns : void
{
    free(bf->store);
    free(bf);
}

void BinFileEncryptAdd(BINFILE *bf,byte a)
// adds a to every byte in the binfile, carry is lost.
{
    int x;
    for (x=0;x<bf->bytes;x++) bf->store[x]+=a;
}

void BinFileEncryptCarryAdd(BINFILE *bf,byte a)
// adds a to every byte in the binfile, carries remainder of byte add to
// the next byte add, when carry == 0, starts process again at current
// unencrypted byte
{

}

void BinFileLoopAdd(BINFILE *bf,byte a,byte step)
// initially adds a to the first byte, then successively adds step to a
// and a to each byte in the binfile, carry is lost.
{
    int x;
    byte set=a;
    for (x=0;x<bf->bytes;x++)
    {
        bf->store[x]+=set;
        set+=step;
    }
}


void BinFileEncryptShift(BINFILE *bf,int bytes)
// shifts the entire binfile by bytes, +ve is right, -ve is left;
{

}

void BinFileEncryptOddEven(BINFILE *bf)
// odd and even bits are swapped
{

}

void BinFileEncryptString(BINFILE *bf,char *string,int direction)
// adds the string byte by byte to each string lengthed section of the
// binfile. subtracts if direction <0 else adds
{
//    int ssize=strlen(string);
}

void BinFileEncryptInvert(BINFILE *bf)
// ones become zeros and zeros become ones.
{
    int x;
    byte mask=255; // 11111111
    for (x=0;x<bf->bytes;x++) bf->store[x]=bf->store[x]^mask;    
}

void BinFileEncryptExpand(BINFILE *bf,int direction)
// if direction>0 a random bit is inserted after each bit 1111 -> 10101110
// else every second bit is removed 10101110 -> 1111 
{

}


/*
void main()
{
    char buffer[20];
    byte ch;
    int i,read;
    BINFILE *bf=BinFileNew(40);

    BinFilePut(bf,11,"Hello World");
    BinFilePutInt(bf,52);
    BinFileWriteFull(bf,"HELLO.DAT");
    BinFileFree(bf);

    bf=BinFileReadFull("HELLO.DAT",5);
    BinFilePrint(bf);

    BinFileGet(bf,11,buffer);
    i=BinFileGetInt(bf);
    printf("'%s' '%d'\n",buffer,i);
    printf("---------------\n");
    BinFileEncryptAdd(bf,190);
    BinFileEncryptAdd(bf,-190);
    BinFilePrint(bf);            
    BinFileEncryptInvert(bf);
    BinFileEncryptInvert(bf);
    BinFilePrint(bf);            
    BinFileLoopAdd(bf,0,100);
    BinFileLoopAdd(bf,0,-100);
    BinFilePrint(bf);            
    BinFileFree(bf);
}
*/
