/* Declarations the extracted NINARO.C functions need. */
#ifndef HARNESS_H
#define HARNESS_H

#include <stdlib.h>
#include "sys.h"
#include "grid.h"
#include "player.h"
#include "connectn.h"

extern GRID *playgrid; /* NINARO.C global used by RuleMove */
extern WIN *win;       /* NINARO.C global used by RuleMove */

int RandomMoveFromList(int size, int *list);
int RuleMove(GRID *grid, int level, PLAYER *thisplayer, PLAYER *otherplayer);

/* Watcom C rand(), seeded per game. */
void WatcomSrand(unsigned long seed);

#endif
