/****************************************************************
Copyright 1998 the author
Module Name    : ENTITY
Module Purpose : to design AI players
Creation Date  : 26 April 1998
Updated        : None
****************************************************************/
#include "connectn.h"
#include "grid.h"

int RandomMove(GRID *grid,TOKEN *last,int skill)
{
    col=random of cols
    if col full
        free = cols not full
        move = (random of free)'th free column
        return move
}

void RuleMove(GRID *grid,TOKEN *last,int skill)
{
    skill = low
    {
    if () SetAdd(n)
    if () SetAdd(n)
    if () SetAdd(n)
    if () SetAdd(n)
    move = random of (free cols as per set)
    }
    skill = high
    assign all cols a value.
    rules are run and values are changed accordingly
    move = random of highest of values
}

void MiniMaxMove(GRID *grid,TOKEN *last,int skill)
{
    horizon = skill level
}

void ABPruneMove(GRID *grid,TOKEN *last,int skill)
{
    horizon = skill level
}
