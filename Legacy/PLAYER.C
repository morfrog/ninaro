#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "player.h"

void PlayerReset(PLAYER *player,char *name,int color,int type,int skill)
{
    player->color=color;
    player->score=0;
    player->type=type;
    player->skill=skill;
    strcpy(player->name,name);
}

PLAYER *PlayerNew(char *name,int color,int type,int skill)
{
    PLAYER *ret=malloc(sizeof(PLAYER));
    PlayerReset(ret,name,color,type,skill);
    return ret;
}

void PlayerIncScore(PLAYER *player)
{
    (player->score)++;
}

void PlayerDecScore(PLAYER *player)
{
    (player->score)++;
}

void PlayerChangeScore(PLAYER *player,int amount)
{
    player->score+=amount;    
}

void PlayerChangeColor(PLAYER *player,int color)
{
    player->color+=color;
}

int PlayerGetColor(PLAYER *player)
{
    return (player->color);
}

int PlayerGetScore(PLAYER *player)
{
    return (player->score);
}

int PlayerGetType(PLAYER *player)
{
    return (player->type);
}

void PlayerSetType(PLAYER *player,int type)
{
    player->type=type;
}

char *PlayerGetName(PLAYER *player)
{
    return (player->name);
}

int PlayerGetSkill(PLAYER *player)
{
    return (player->skill);
}

int PlayerGroupFind(PLAYERGROUP *pg,PLAYER *pl)
{
    int x;
    for (x=0;x<pg->players;x++)
       if (pg->player[x]==pl) return x; // found
    return -1; // not found
}

PLAYERGROUP *PlayerGroupNew(int players,int groupsize,...)
// creates a new player group containing groupsize players, with players
// players able to be active at any one time
// ... is groupsize*{PLAYER *}
{
    va_list ap;
    int x;
    PLAYERGROUP *pg=malloc(sizeof(PLAYERGROUP));
    pg->players=groupsize;
    pg->player=malloc(groupsize*sizeof(PLAYER *));
    pg->currentplayers=players;
    pg->currentplayer=malloc(players*sizeof(int));
    pg->current=0;
    va_start(ap,groupsize);
    for (x=0;x<groupsize;x++)
    {
        pg->player[x]=va_arg(ap,PLAYER *);
    }
    va_end(ap);
    for (x=0;x<players;x++) pg->currentplayer[x]=x;
    return pg;
}

PLAYER *PlayerGroupPlayer(PLAYERGROUP *pg,int player)
// returns the player'th player in the group
{
    return pg->player[player];
}

void PlayerGroupUseSet(PLAYERGROUP *pg,int pnum,int player)
// sets the pmun player in use to player
{
    pg->currentplayer[pnum]=player;
}

PLAYER *PlayerGroupUseGet(PLAYERGROUP *pg,int pnum)
// returns the pnum player in use
{
    return pg->player[pg->currentplayer[pnum]];
}

void PlayerGroupCurrentSet(PLAYERGROUP *pg,int pnum)
// for a two player in use group, pnum = 0..1
{
    pg->current=pnum;    
}

void PlayerGroupCurrentNext(PLAYERGROUP *pg)
{
    if (pg->current>=pg->currentplayers-1)
        pg->current=0;
    else
        pg->current+=1;
}

PLAYER *PlayerGroupCurrentGet(PLAYERGROUP *pg)
{
    return pg->player[pg->currentplayer[pg->current]];
}

void PlayerGroupFree(PLAYERGROUP *pg)
{
    free(pg->player);
    free(pg->currentplayer);    
}

