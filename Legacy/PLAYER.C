#include <stdlib.h>
#include <string.h>
#include "player.h"

void ResetPlayer(PLAYER *player,char *name,int color,int human)
{
    player->color=color;
    player->score=0;
    player->human=human;
    strcpy(player->name,name);
}

PLAYER *NewPlayer(char *name,int color,int human)
{
    PLAYER *ret=malloc(sizeof(PLAYER));
    ResetPlayer(ret,name,color,human);
    return ret;
}

void IncPlayerScore(PLAYER *player)
{
    (player->score)++;
}

void DecPlayerScore(PLAYER *player)
{
    (player->score)++;
}

void ChangePlayerScore(PLAYER *player,int amount)
{
    player->score+=amount;    
}

void ChangePlayerColor(PLAYER *player,int color)
{
    player->color=color;
}

int GetPlayerColor(PLAYER *player)
{
    return (player->color);
}

int GetPlayerScore(PLAYER *player)
{
    return (player->score);
}

int IsHuman(PLAYER *player)
{
    return (player->human);
}

char *GetPlayerName(PLAYER *player)
{
    return (player->name);
}
