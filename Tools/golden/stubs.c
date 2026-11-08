/* Stand-ins for the parts of SYS.C and SCREEN.C that the grid, win check,
 * player and binfile modules call. SYS.C and SCREEN.C need DOS and Watcom's
 * graph.h, so they aren't compiled; the functions below are copied from them
 * where they matter (IntToByteString, ByteStringToInt, Random). */
#include <stdio.h>
#include "harness.h"

/* Watcom C/386 rand(): the ANSI C example generator, RAND_MAX 32767. */
static unsigned long next = 1;

void WatcomSrand(unsigned long seed) { next = seed; }

static int WatcomRand(void)
{
    next = next * 1103515245UL + 12345UL;
    return (int)((next >> 16) & 0x7FFF);
}

/* SYS.C Random */
int Random(int num) { return (WatcomRand() % num); }

/* SYS.C IntToByteString, unchanged */
void IntToByteString(int in, char *array)
{
    int mask = 255;
    int masker, x;
    for (x = 0; x < 4; x++)
    {
        masker = mask << ((3 - x) * 8);
        array[x] = (masker & in) >> ((3 - x) * 8);
    }
}

/* SYS.C ByteStringToInt, unchanged (built with /J: Watcom's char is unsigned) */
int ByteStringToInt(char *array)
{
    int ret = 0, x;
    for (x = 0; x < 4; x++)
        ret += (array[x] << ((3 - x) * 8));
    return ret;
}

/* SYS.C FileSize used DOS directory calls; this gives the same answer. */
long FileSize(char *directory, char *filename)
{
    long size;
    FILE *f = fopen(filename, "rb");
    (void)directory;
    if (f == NULL) return 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);
    return size;
}

/* SCREEN.C drawing, not needed here. */
void DrawGrid(int x, int y) { (void)x; (void)y; }
void DrawDisc(int col, int cols, int row, int rows, int color, double discsize)
{
    (void)col; (void)cols; (void)row; (void)rows; (void)color; (void)discsize;
}
