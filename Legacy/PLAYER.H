/****************************************************************
Copyright 1998 the author
Module Name    : PLAYER
Module Purpose : to provide a player object
Creation Date  : 2 March 1998
Updated        : 12 April 1998  CREATED PlayerGroup's
                 2 MAY 1998     ADDED getmove func to player
****************************************************************/
#ifndef PLAYER_H
#define PLAYER_H

enum {Computer,Human}; // Player type

struct playerTag
{
    char name[20];
    int color;  // 0..15
    int type;   // {Computer,Human}
    int pid;    // internal player id, used by playergroup
    int (* getmove)(void *); // function which gets the players move
};
typedef struct playerTag PLAYER;

struct playergroupTag
{
    int players;        // number of players in group
    PLAYER **player;    // storage of the players
};
// could have 7 players in group, of which 2 (3,4) are in use of which 0 (3)
// is the current player
typedef struct playergroupTag PLAYERGROUP;

PLAYER *PlayerNew(char *name,int color,int type,int (* getmove)(void *));
// purpose : to create a new player
// returns : the player
void PlayerReset(PLAYER *player,char *name,int color,int type,int (* getmove)(void *));
// purpose : to reset a players attributes
// returns : void
int  PlayerGetColor(PLAYER *player);
void PlayerSetColor(PLAYER *player,int color);
int  PlayerGetType(PLAYER *player);
void PlayerSetType(PLAYER *player,int type);
char *PlayerGetName(PLAYER *player);
void PlayerSetName(PLAYER *player,char *name);

PLAYERGROUP *PlayerGroupNew(int players,...);
// purpose : creates a new player group containing players
//           ... is players*{PLAYER *}
// returns : the player group
PLAYER *PlayerGroupPlayer(PLAYERGROUP *pg,int index);
// purpose : to return player in the at index in the group
// returns : NULL if index out of range, else the player *
int PlayerGroupFind(PLAYERGROUP *pg,PLAYER *pl);
// purpose : to find the index of player in the playergroup
// returns : -1 if player not in group else index is returned
void PlayerGroupFree(PLAYERGROUP *pg);
// purpose : to free memory allocated to the playergroup
// returns : void

#endif
