/****************************************************************
Copyright 1998 the author
Module Name    : SOUND
Module Purpose : to provide PC speaker sound facilities
Creation Date  : 18 March 1998
Updated        : 24 April 1998       ADDED Mute functions
****************************************************************/
#ifndef SOUND_H
#define SOUND_H

int ToggleMute(void);
int IsMute();
void Mute(int mute);
void Buzz(int pitch1,int pitch2,int pitchhold,int length);
void PlayTune(int notes, ...); // note,length pairs

#endif
