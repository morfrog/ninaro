/****************************************************************
Copyright 1998 the author
Module Name    : PLAYER
Module Purpose : to provide a player object
Creation Date  : 2 March 1998
Updated        : 12 April 1998  CREATED PlayerGroup's
                 2 MAY 1998     ADDED getmove func to player
                 6 May 1998     CREATED PlayerCopy
****************************************************************/
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "player.h"

void PlayerReset(PLAYER *player,char *name,int color,int type,int (* getmove)(void *))
// purpose : to reset a players attributes
// returns : void
{
    strcpy(player->name,name);
    player->color=color;
    player->type=type;
    player->getmove=getmove;
}

void PlayerCopy(PLAYER *dest,PLAYER *source)
// purpose : to copy one source players attributes to dest players attributes
// returns : void
{
    strcpy(dest->name,source->name);
    dest->color=source->color;
    dest->type=source->type;
    dest->getmove=source->getmove;
}

PLAYER *PlayerNew(char *name,int color,int type,int (* getmove)(void *))
// purpose : to create a new player
// returns : the player
{
    PLAYER *ret=malloc(sizeof(PLAYER));
    PlayerReset(ret,name,color,type,getmove);
    return ret;
}

void PlayerFree(PLAYER *player)
// purpose : to free the memory allocated to a player
// returns : void
{
    free(player);
}

int  PlayerGetColor(PLAYER *player) {return player->color;}
// purpose : to get player's color
// returns : player's color

void PlayerSetColor(PLAYER *player,int color) {player->color=color;}
// purpose : to set player's color
// returns : void

int  PlayerGetType(PLAYER *player) {return player->type;}
// purpose : to get player's type
// returns : player's type

void PlayerSetType(PLAYER *player,int type) {player->type=type;}
// purpose : to set player's type
// returns : void

char *PlayerGetName(PLAYER *player) {return player->name;}
// purpose : to get player's name
// returns : player's name

void PlayerSetName(PLAYER *player,char *name) {strcpy(player->name,name);}
// purpose : to set player's name
// returns : void

int PlayerGroupFind(PLAYERGROUP *pg,PLAYER *pl)
// purpose : to find the pid of player in the playergroup
// returns : -1 if player not in group else index is returned
{
    return pl->pid;
/*
    int x;
    for (x=0;x<pg->players;x++)
       if (pg->player[x]==pl) return x; // found
    return -1; // not found
*/
}

PLAYERGROUP *PlayerGroupNew(int players,...)
// purpose : creates a new player group containing players
//           ... is players*{PLAYER *}
// returns : the player group
{
    va_list ap;
    int x;
    PLAYERGROUP *pg=malloc(sizeof(PLAYERGROUP));
    pg->players=players;
    pg->player=malloc(players*sizeof(PLAYER *));
    va_start(ap,players);
    for (x=0;x<players;x++)
    {
        pg->player[x]=va_arg(ap,PLAYER *);
        pg->player[x]->pid=x;
    }
    va_end(ap);
    return pg;
}

PLAYER *PlayerGroupPlayer(PLAYERGROUP *pg,int index)
// purpose : to return player in the at index in the group
// returns : NULL if index out of range, else the player *
{
    if (index>=pg->players) return NULL;
    return pg->player[index];
}

void PlayerGroupFree(PLAYERGROUP *pg)
// purpose : to free memory allocated to the playergroup
// returns : void
{
    free(pg->player);
    free(pg);
}

