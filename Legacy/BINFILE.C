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
    bf->atbyte=0;
    bf->store=malloc(bytes*sizeof(byte));
    for (x=0;x<bytes;x++) bf->store[x]=0;
    return bf;
}

BINFILE *BinFileRead(char *filename)
// purpose : reads a bin file from a file
// returns : the binfile, else NULL if file does not exist
{
    FILE *fp;
    int size=15; // (int)FileSize(".",filename);
    BINFILE *bf;

    if (size==-1) return NULL; // file does not exist
    bf=BinFileNew(size);
    fp=fopen(filename,"r");  // assume exists because size!=-1;
    if (fp==NULL) exit(0);
    printf("Read %u bytes.\n",fread(bf->store,sizeof(byte),(size_t)size,fp));
    bf->atbyte=0;
    fclose(fp);
    return bf;
}

int     BinFileWrite(BINFILE *bf,char *filename)
// purpose : write a binfile to a file
// returns : TRUE if successful else false 
{
    FILE *file;

    file=fopen(filename,"wb");
    if (file==NULL) return FALSE;

    fwrite(bf->store,sizeof(byte),bf->atbyte,file);
    fclose(file);
    return TRUE;
}

int BinFileRemain(BINFILE *bf)
// purpose : how many bytes are left to be put to or got from the binfile
// returns : the integer
{
    return ((bf->size)-(bf->atbyte));
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
            bf->store[bf->atbyte]=element[x];
            bf->atbyte+=1;
        }
        return TRUE;
    }
}

int BinFilePutBinfile(BINFILE *target,BINFILE *source)
// purpose : adds source binfile to the end of target binfile
// returns : TRUE is successful, FALSE otherwise
{
    if (BinFileRemain(target)<source->atbyte)
        return FALSE;
    else
        return BinFilePut(target,source->atbyte,source->store);
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
        element[x]=bf->store[bf->atbyte];
        bf->atbyte+=1;
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
        element[x]=bf->store[bf->atbyte];
        bf->atbyte+=1;
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
    printf("Size = %d \nAtByte = %d\n'",bf->size,bf->atbyte);
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

void BinFileEncryptAdd(BINFILE *bf,int a)
// adds a to every byte in the binfile, carry is lost.
{
    int x;
    for (x=0;x<
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

}

void BinFileEncryptInvert(BINFILE *bf)
// ones become zeros and zeros become ones.
{

}

void BinFileEncryptExpand(BINFILE *bf,int direction)
// if direction>0 a random bit is inserted after each bit 1111 -> 10101110
// else every second bit is removed 10101110 -> 1111 
{

}
*/

/*
void main()
{
    char buffer[20];
    int i;
    BINFILE *bf=BinFileNew(40);

    BinFilePut(bf,11,"Hello World");
    BinFilePutInt(bf,52);
    BinFileWrite(bf,"HELLO.DAT");
    BinFileFree(bf);

    bf=BinFileRead("HELLO.DAT");
    BinFilePrint(bf);

    BinFileGet(bf,11,buffer);
    i=BinFileGetInt(bf);
    printf("'%s' '%d'\n",buffer,i);
    BinFileFree(bf);
}
*/
