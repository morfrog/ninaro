#ifndef PLAYER_H

struct playerTag
{
        char name[18];
        int color;
        int score;
        int human;
};

typedef struct playerTag PLAYER;

PLAYER *NewPlayer(char *name,int color,int human);
void IncPlayerScore(PLAYER *player);
void DecPlayerScore(PLAYER *player);
void ChangePlayerScore(PLAYER *player,int amount);
void ResetPlayer(PLAYER *player,char *name,int color,int human);
void ChangePlayerColor(PLAYER *player,int color);
int GetPlayerColor(PLAYER *player);
int GetPlayerScore(PLAYER *player);
char *GetPlayerName(PLAYER *player);
int IsHuman(PLAYER *player);

#endif
