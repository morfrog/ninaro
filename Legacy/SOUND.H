#ifndef SOUND_H

int ToggleMute(void);
int IsMute();
void Mute(int mute);
void Buzz(int pitch1,int pitch2,int pitchhold,int length);
void PlayTune(int notes, ...); // note,length pairs

#endif
