#ifndef PLAYER_H
#define PLAYER_H

enum {Computer,Human}; // Player type

enum {Low,Medium,High}; // Skill Levels
// Humans are 'rated' with a skill level too

struct playerTag
{
        char name[20];
        int color;  // 0..15
        int score;  // 0..
        int type;   // {Computer,Human}
        int skill;  // {Low,Medium,High}
};

typedef struct playerTag PLAYER;

struct playergroupTag
{
    int players;        // number of players in group
    PLAYER **player;    // storage of the players
    int currentplayers;// number of players who are in use 0..players-1
    int *currentplayer;// storage of the players in use
    int current;       // the current player of those in use; 0..currentplayers-1
};
// could have 7 players in group, if which 2 (3,4) are in use of which 0 (3)
// is the current player

typedef struct playergroupTag PLAYERGROUP;

PLAYER *PlayerNew(char *name,int color,int type,int skill);
void PlayerIncScore(PLAYER *player);
void PlayerDecScore(PLAYER *player);
void PlayerChangeScore(PLAYER *player,int amount);
void PlayerReset(PLAYER *player,char *name,int color,int type,int skill);
void PlayerChangeColor(PLAYER *player,int color);
int PlayerGetColor(PLAYER *player);
int PlayerGetScore(PLAYER *player);
char *PlayerGetName(PLAYER *player);
int PlayerGetSkill(PLAYER *player);
int PlayerGetType(PLAYER *player);
void PlayerSetType(PLAYER *player,int type);

PLAYERGROUP *PlayerGroupNew(int players,int groupsize,...);
PLAYER *PlayerGroupPlayer(PLAYERGROUP *pg,int player);
void PlayerGroupUseSet(PLAYERGROUP *pg,int pnum,int player);
PLAYER *PlayerGroupUseGet(PLAYERGROUP *pg,int pnum);
void PlayerGroupCurrentSet(PLAYERGROUP *pg,int pnum);
void PlayerGroupCurrentNext(PLAYERGROUP *pg);
PLAYER *PlayerGroupCurrentGet(PLAYERGROUP *pg);
void PlayerGroupFree(PLAYERGROUP *pg);

#endif
