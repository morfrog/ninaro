/* Golden-game harness for the Ninaro web port.
 *
 * Plays seeded AI-vs-AI games with the 1998 grid, win-check and AI code
 * (Legacy GRID.C, CONNECTN.C, PLAYER.C, BINFILE.C, and RuleMove copied from
 * NINARO.C) and writes every move to JSON. The TypeScript port must replay
 * each game exactly (WebApp/src/core/golden.test.ts).
 *
 * The game loop follows NINARO.C main() and DropToken() for two AIs at the
 * "Fastest" speed: the current player moves, a win ends the game, a full
 * board after a successful move is a draw, otherwise play passes over.
 *
 * Usage: harness <Legacy/NINARO.TTL> > golden.json
 */
#include <stdio.h>
#include "harness.h"

GRID *playgrid;
WIN *win;

struct board { int cols, rows, connect; };

/* The menu boards (7x8x4, 12x13x6), the smallest allowed (3x3x3), and
 * shapes that test edges, tall and wide boards, and long connects. */
static const struct board BOARDS[] = {
    {7, 8, 4}, {12, 13, 6}, {3, 3, 3}, {5, 4, 3},
    {10, 6, 5}, {4, 9, 4}, {8, 3, 8}, {20, 15, 5},
};
#define NBOARDS (sizeof(BOARDS) / sizeof(BOARDS[0]))
#define LEVELS 5 /* AI Random, Novice, Normal, Expert, Genius = rules 0..4 */
#define SEEDS 8

static void PlayGame(const struct board *b, int level1, int level2, unsigned long seed, int first)
{
    PLAYER *p[2];
    int level[2], current = first, moves = 0, col, ret, x;
    const char *result = "draw";

    p[0] = PlayerNew("One", 1, Computer, NULL);
    p[1] = PlayerNew("Two", 2, Computer, NULL);
    level[0] = level1;
    level[1] = level2;

    WatcomSrand(seed);
    playgrid = GridNew(b->cols, b->rows);
    win = malloc(sizeof(WIN));
    win->win = NULL;
    SetWinSize(win, b->connect);

    printf("{\"cols\":%d,\"rows\":%d,\"connect\":%d,\"levels\":[%d,%d],\"seed\":%lu,\"first\":%d,\"moves\":\"",
           b->cols, b->rows, b->connect, level1, level2, seed, first);
    for (;;)
    {
        col = RuleMove(playgrid, level[current], p[current], p[1 - current]);
        if (col < 0) { result = "nomove"; break; }
        ret = InsertTokenInCol(playgrid, col, p[current], win);
        printf(moves ? " %d" : "%d", col);
        ++moves;
        if (ret == Win) { result = current == 0 ? "one" : "two"; break; }
        if (ret == Failure) { result = "failure"; break; }
        if (TokensInGrid(playgrid) == MaxTokensInGrid(playgrid)) break; /* draw */
        current = 1 - current;
    }
    printf("\",\"result\":\"%s\",\"win\":[", result);
    if (ret == Win)
        for (x = 0; x < win->tokens; x++)
            printf("%s[%d,%d]", x ? "," : "", win->win[x].col, win->win[x].row);
    printf("]}");

    free(win->win);
    free(win);
    GridFree(playgrid);
    PlayerFree(p[0]);
    PlayerFree(p[1]);
}

/* The title picture, loaded with GridLoadFull as TitleScreen() did: each
 * cell is 0 (empty) or the player group index + 1. */
static void DumpTitle(const char *path)
{
    PLAYERGROUP *pg = PlayerGroupNew(8,
        PlayerNew("0", 0, Human, NULL), PlayerNew("1", 0, Human, NULL),
        PlayerNew("2", 0, Computer, NULL), PlayerNew("3", 0, Computer, NULL),
        PlayerNew("4", 0, Computer, NULL), PlayerNew("5", 0, Computer, NULL),
        PlayerNew("6", 0, Computer, NULL), PlayerNew("7", 0, Computer, NULL));
    GRID *g = GridLoadFull(pg, (char *)path);
    int x, y;
    PLAYER *pl;
    if (g == NULL) { fprintf(stderr, "Can't read %s\n", path); exit(1); }
    printf("\"title\":{\"cols\":%d,\"rows\":%d,\"tokens\":%d,\"cells\":[", GridCols(g), GridRows(g), TokensInGrid(g));
    for (x = 0; x < GridCols(g); x++)
    {
        printf("%s\"", x ? "," : "");
        for (y = 0; y < GridRows(g); y++)
        {
            pl = StatusOfCR(g, x, y);
            printf("%d", pl == NULL ? 0 : PlayerGroupFind(pg, pl) + 1);
        }
        printf("\"");
    }
    printf("]}");
    GridFree(g);
}

int main(int argc, char **argv)
{
    unsigned b, l1, l2, s, n = 0;
    if (argc < 2) { fprintf(stderr, "Usage: harness NINARO.TTL\n"); return 1; }
    printf("{\"generator\":\"Tools/golden/harness.c\",");
    DumpTitle(argv[1]);
    printf(",\"games\":[\n");
    for (b = 0; b < NBOARDS; b++)
        for (l1 = 0; l1 < LEVELS; l1++)
            for (l2 = 0; l2 < LEVELS; l2++)
                for (s = 1; s <= SEEDS; s++)
                {
                    if (n++) printf(",\n");
                    PlayGame(&BOARDS[b], (int)l1, (int)l2, s * 7919UL + b, (int)(s % 2));
                }
    printf("\n]}\n");
    return 0;
}
