#ifndef KEY_H
#define KEY_H
#include "set.h"

// prefixes : S=SHIFT C=CTRL A=ALT KP=keypad(numlock off)
enum {BKSP=8,SENTER=10,ENTER=13,ESC=27,SPACE=32,DEL=127}; // Normal Keys
enum {Cat=0 /*=C@*/,CA,CB,CC,CD,CE,CF,CG,CH,CI,CJ,CK,CL,CM,CN,CO,CP,CQ,CR,
      CS,CT,CU,CV,CW,CX,CY,CZ,
      F1=256+59,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11=256+133,F12,// Function Keys
      KP7=256+71,KP8,KP9,KP4=256+75,KP5,KP6,KP1=256+79,KP2,KP3,KP0,KPDEL,
                                                            // Keypad Keys
      SF1=256+84,SF2,SF3,SF4,SF5,SF6,SF7,SF8,SF9,SF10,      // Shift F Keys
      CF1,CF2,CF3,CF4,CF5,CF6,CF7,CF8,CF9,CF10,             // Ctrl F Keys
      AF1,AF2,AF3,AF4,AF5,AF6,AF7,AF8,AF9,AF10};            // Alt F Keys

struct keygroupTag
{
    int keys;       // number of keys in group
    int *key;       // array of keys
    SET *ingroup;   // set of keys in group
};
typedef struct keygroupTag KEYGROUP;

struct keyfuncTag
{
    int (* func)(void *);   // array of (function to run for key)    
};
typedef struct keyfuncTag KEYFUNC;

struct keysysTag
{
    KEYGROUP *kg;       // pointer to key group
    SET *enabled;       // array of (is key [x] enabled)
    KEYFUNC *kf;        // array of key functions
};
typedef struct keysysTag KEYSYS;


/*-----------------------------KEY----------------------------------*/
int GetKey();
// purpose : gets a key from console, of type enum'd in key.h
// returns : the key
char *KeyString(int key);               // not yet implemented
int KeyIsAlt(int key);                  // not yet implemented
int KeyIsShift(int key);                // not yet implemented
int KeyIsCtrl(int key);                 // not yet implemented
int KeyIsNormal(int key);               // not yet implemented

/*--------------------------KEYGROUP--------------------------------*/
KEYGROUP *KeyGroupNew(int keys,...);
// purpose : create a new key group, ... is {int}*keys e.g. CA,CB,KP4
// returns : the key group
int KeyGroupIKey(KEYGROUP *kg,int index);
// purpose : to get the key at index in the keygroup
// returns : -1 if index out of range else the key us returned
int KeyGroupKey(KEYGROUP *kg,int key);
// purpose : to get the number of the key in the group, 0..keys-1,
// returns : -1 if key not in group else the keys index(0..keys-1)
void KeyGroupReset(KEYGROUP *kg,...);
// purpose : resets the keys in the keygroup, ... is {int}*kg->keys
// returns : void
void KeyGroupCopy(KEYGROUP *dest,KEYGROUP *source);
// purpose : to copy the key contents of source to destination, source must
//           have at least as many keys as dest.
// returns : void
void KeyGroupPrint(KEYGROUP *kg);
// purpose : print the contents of the keygroup
// returns : void

/*-------------------------KEYSYS-----------------------------------*/
KEYSYS *KeySysNew(KEYGROUP *kg);
// purpose : create a new keysys. must use KeySysFunc to initialise functions
// returns : the keysys
void KeySysIFunc(KEYSYS *ks,int index,int (* func)(void *));
// purpose : adds func to the keysys, at index
// returns : void
int KeySysFunc(KEYSYS *ks,int key,int (* func)(void *));
// purpose : adds func to key in keysys
// returns : TRUE if added else FALSE
int KeySysIKey(KEYSYS *ks,int index,void *object,int *funcret);
// purpose : runs function at index of keysys on object
// returns : TRUE if index in range else FALSE
int KeySysKey(KEYSYS *ks,int key,void *object,int *funcret,int *index);
// purpose : to handle the input of key, runs associated function on object
//           if key is enabled. sets funcret to result of running keys
//           function, index is -1 if key not in ks, else index is keys index 
// returns : TRUE if key was used, else FALSE
void KeySysIEnable(KEYSYS *ks,int index,int enable);
// purpose : to set the enabled state of the index'th key in the keysys
// returns : void
int KeySysEnable(KEYSYS *ks,int key,int enable);
// purpose : to set the enabled state of the key in the keysys
// returns : FALSE if key is not valid, else TRUE and the key is enabled
void KeySysGroupSet(KEYSYS *ks,KEYGROUP *kg);
// purpose : to set the keysys keygroup
// returns : void
KEYGROUP *KeySysGroupGet(KEYSYS *ks);
// purpose : to get the keysys keygroup
// returns : the keygroup
void KeySysPrint(KEYSYS *ks);
// purpose : prints the contents of the keysys
// returns : void
void KeySysFree(KEYSYS *ks);
// purpose : to free memory allocated to the keysys
// returns : void

#endif
