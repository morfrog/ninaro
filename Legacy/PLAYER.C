#include <stdlib.h>
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

