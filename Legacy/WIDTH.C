#include "binfile.h"

#define wide 60

void main()
{
    int line,got;
    BINFILE *in=BinFileReadFull("NINARO.TXT",0);
    BINFILE *out=BinFileNew(BinFileRemain(in)+500);
    char buffer[2];

    do
    {
        got=BinFileGet(in,1,buffer);
        if (buffer[0]==13)  // skip carriage return
        {
            got=BinFileGet(in,1,buffer); // skip line feed
            buffer[0]=' ';
            BinFilePut(out,1,buffer);    // put space
            got=BinFileGet(in,1,buffer); // get character
        }
        BinFilePut(out,1,buffer);
    } while (BinFileRemain(in)>0);
    BinFileWriteFull(out,"NINARO.TEX");
}
