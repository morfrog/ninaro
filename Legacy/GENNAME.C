/****************************************************************
Copyright 1998 the author
Module Name    : GENNAME
Module Purpose : to generate the save file for the ninaro title screen
Creation Date  : 23 April 1998
Updated        : 3 May 1998         update sizes for visiblility reasons
****************************************************************/

#include <stdlib.h>

#include "binfile.h"
#define COLS 36
#define ROWS 7

void main()
{
    int array[ROWS][COLS]={
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,2,2,1,1},
    {1,2,1,1,2,2,1,1,2,1,1,2,1,1,2,2,1,1,2,1,1,2,1,1,2,1,2,1,1,1,2,1,1,1,2,1},
    {1,2,1,2,1,2,1,1,2,1,1,2,1,2,1,2,1,1,2,2,2,2,1,1,2,2,2,1,1,1,2,1,1,1,2,1},
    {1,2,2,1,1,2,1,1,2,1,1,2,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1},
    {1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};
    int x,y;
    BINFILE *bf=BinFileNew(sizeof(int)*(2+(ROWS*COLS)));

    BinFilePutInt(bf,COLS); // width
    BinFilePutInt(bf,ROWS);  // height

    for (x=0;x<COLS;x++)
        for (y=0;y<ROWS;y++)
        {
            BinFilePutInt(bf,array[y][x]);
        }    
    
    BinFileWriteFull(bf,"NINARO.TTL");
}
