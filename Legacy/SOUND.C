#include <i86.h>
#include <stdarg.h>
#include "defines.h"
int _mute=FALSE;

void Mute(int mute)
{
    _mute=mute;
}

int IsMute()
{
    return _mute;
}

int ToggleMute(void)
// returns true if now mute, else FALSE;
{
    if (_mute) _mute=FALSE; else _mute=TRUE;
    return _mute;
}

void Buzz(int pitch1,int pitch2,int pitchhold,int length)
{
    int hold=pitchhold/2;
    int done=0;
    int ever=1;
    if (_mute) return;
    for (;ever;)
    {
        sound(pitch1);
        delay(hold);
        sound(pitch2);
        delay(hold);
        done+=pitchhold;
        if (done>length) ever=0;
    }
    nosound();
}

void PlayTune(int notes, ...) // note,length pairs
{
    va_list args;
    int note,length,x;

    if (_mute) return;

    va_start(args,notes);
    for (x=0;x<notes;x++)
    {
        note=va_arg(args,int);
        length=va_arg(args,int);
        sound(note);
        delay(length);
    }
    va_end(args);
    nosound();
}
