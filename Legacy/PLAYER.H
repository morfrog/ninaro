#ifndef PLAYER_H
#define PLAYER_H

enum {Computer,Human}; // Player type

enum {Low,Medium,High}; // Skill Levels
// Humans are 'rated' with a skill level too

struct playerTag
{
        char name[20];
        int color;
        int score;
        int human;
        int skill;
};

typedef struct playerTag PLAYER;

PLAYER *PlayerNew(char *name,int color,int human,int skill);
void PlayerIncScore(PLAYER *player);
void PlayerDecScore(PLAYER *player);
void PlayerChangeScore(PLAYER *player,int amount);
void PlayerReset(PLAYER *player,char *name,int color,int human,int skill);
void PlayerChangeColor(PLAYER *player,int color);
int PlayerGetColor(PLAYER *player);
int PlayerGetScore(PLAYER *player);
char *PlayerGetName(PLAYER *player);
int PlayerGetSkill(PLAYER *player);
int PlayerIsHuman(PLAYER *player);

#endif
