#include <stdlib.h>
#include <stdio.h>
#include <conio.h> // getch
#include <stdarg.h>
#include "defines.h"
#include "key.h"

int GetKey()
// purpose : gets a key from console, of type enum'd in key.h
// returns : the key
{
    int ch=getch();
    if (ch==0)
    {
        return (256+getch());
    }
    else return ch;
}

void KeyPrint(int key)
// purpose : print the key, extended key <0:num>, normal <num>
{
    printf("<%s%d>",key>255?"0:":"  ",key%256);
}

KEYGROUP *KeyGroupNew(int keys,...)
// purpose : create a new key group, ... is {int}*keys e.g. CA,CB,KP4
// returns : the key group
{
    KEYGROUP *ret=malloc(sizeof(KEYGROUP));
    int param,x;
    va_list ap;

    ret->keys=keys;
    ret->key=malloc(keys*sizeof(int));
    ret->ingroup=SetNew(512); // covers normal + extended keys
    va_start(ap,keys);
    for (x=0;x<keys;x++)
    {
        param=va_arg(ap,int);
        ret->key[x]=param;
        SetAdd(ret->ingroup,param);
    }
    va_end(ap);
    return ret;
}

void KeyGroupCopy(KEYGROUP *dest,KEYGROUP *source)
// purpose : to copy the key contents of source to destination, source must
//           have at least as many keys as dest.
// returns : void
{
    int x;
    SetEmpty(dest->ingroup);
    for (x=0;x<dest->keys;x++)
    {
        dest->key[x]=source->key[x];
        SetAdd(dest->ingroup,dest->key[x]);
    }
}

int KeyGroupIKey(KEYGROUP *kg,int index)
// purpose : to get the key at index in the keygroup
// returns : -1 if index out of range else the key us returned
{
    if (index>=kg->keys) return -1;
    return kg->key[index];
}

int KeyGroupKey(KEYGROUP *kg,int key)
// purpose : to get the number of the key in the group, 0..keys-1,
// returns : -1 if key not in group else the keys index(0..keys-1)
{
    int x;
    if (SetMember(kg->ingroup,key))
    {
        for (x=0;x<kg->keys;x++)
        {
            if (kg->key[x]==key)
                return x;
        }
        return -2; // should never happen;
    }
    return -1; // key not in group
}

void KeyGroupReset(KEYGROUP *kg,...)
// purpose : resets the keys in the keygroup, ... is {int}*kg->keys
// returns : void
{
    va_list ap;
    int x;
    va_start(ap,kg);
    for (x=0;x<kg->keys;x++)
    {
        SetDel(kg->ingroup,kg->key[x]);
        kg->key[x]=va_arg(ap,int);
        SetAdd(kg->ingroup,kg->key[x]);
    }
    va_end(ap);
}

void KeyGroupPrint(KEYGROUP *kg)
// purpose : print the contents of the keygroup
// returns : void
{
    int x;
    for (x=0;x<kg->keys;x++) KeyPrint(kg->key[x]);
    printf("\n");
}

void KeySysIFunc(KEYSYS *ks,int index,int (* func)(void *))
// purpose : adds func to the keysys, at index
// returns : void
{
    ks->kf[index].func=func;
    SetAdd(ks->enabled,index);
}

int KeySysFunc(KEYSYS *ks,int key,int (* func)(void *))
// purpose : adds func to key in keysys
// returns : TRUE if added else FALSE
{
    int index=KeyGroupKey(ks->kg,key);
    if (index==-1) return FALSE;
    KeySysIFunc(ks,index,func);
    return TRUE;
}

KEYSYS *KeySysNew(KEYGROUP *kg)
// purpose : create a new keysys. must use KeySysFunc to initialise functions
// returns : the keysys
{
    KEYSYS *ret=malloc(sizeof(KEYSYS));
    ret->kg=kg;
    ret->enabled=SetNew(kg->keys);
    ret->kf=malloc(kg->keys*sizeof(KEYFUNC));
    
    return ret;
}

int KeySysIKey(KEYSYS *ks,int index,void *object,int *funcret)
// purpose : runs function at index of keysys on object
// returns : TRUE if index in range else FALSE
{
    int *tmp;
    int key=KeyGroupIKey(ks->kg,index);
    if (key==-1) return FALSE;
    return KeySysKey(ks,key,object,funcret,tmp);
}

int KeySysKey(KEYSYS *ks,int key,void *object,int *funcret,int *index)
// purpose : to handle the input of key, runs associated function on object
//           if key is enabled. sets funcret to result of running keys
//           function, index is -1 if key not in ks, else index is keys index 
// returns : TRUE if key was used, else FALSE
{
    *index=KeyGroupKey(ks->kg,key);
    if ((*index)==-1) return FALSE; // key is not in group

    if ((SetMember(ks->enabled,*index)) && (ks->kf[*index].func!=NULL))
    // if enabled and runable
    {
        *funcret=(ks->kf[*index].func(object));
        return TRUE; // key was used
    }
    return FALSE;
}

void KeySysGroupSet(KEYSYS *ks,KEYGROUP *kg)
// purpose : to set the keysys keygroup
// returns : void
{
    ks->kg=kg;
}

KEYGROUP *KeySysGroupGet(KEYSYS *ks)
// purpose : to get the keysys keygroup
// returns : the keygroup
{
    return ks->kg;
}

void KeySysPrint(KEYSYS *ks)
// purpose : prints the contents of the keysys
// returns : void
{
    int x;
    for (x=0;x<ks->kg->keys;x++)
    {
        KeyPrint(ks->kg->key[x]);
        if (SetMember(ks->enabled,x))
            printf("  Enabled");
        else
            printf(" Disabled");
        printf(" %cNULL\n",ks->kf[x].func==NULL?' ':'!');
    }
}

void KeySysIEnable(KEYSYS *ks,int index,int enable)
// purpose : to set the enabled state of the index'th key in the keysys
// returns : void
{
    if (enable)
        SetAdd(ks->enabled,index);
    else
        SetDel(ks->enabled,index);
}

int KeySysEnable(KEYSYS *ks,int key,int enable)
// purpose : to set the enabled state of the key in the keysys
// returns : FALSE if key is not valid, else TRUE and the key is enabled
{
    int index=KeyGroupKey(ks->kg,key);
    if (index==-1) return FALSE;
    KeySysIEnable(ks,index,enable);
    return TRUE;
}

void KeySysFree(KEYSYS *ks)
// purpose : to free memory allocated to the keysys
// returns : void
{
    free(ks->kf);
    SetFree(ks->enabled);
}

/*
int KeyIsAlt(int key)
// purpose : is key an alt key e.g. AF1
// returns : TRUE if alt key else FALSE
{

}

int KeyIsShift(int key)
{

}

int KeyIsCtrl(int key)
{

}

int KeyIsNormal(int key)
{

}

char *KeyString(int key)
// purpose : get the string name of the key
{

}
*/

/*
void main()
{
    KEYGROUP *kg=KeyGroupNew(6,KP4,KP6,KP8,KP2,ENTER,ESC);
    KEYSYS *ks=KeySysNew(kg);

    KeySysIFunc(ks,0,NULL);
    KeySysIFunc(ks,1,NULL);
    KeySysIFunc(ks,2,NULL);
    KeySysIFunc(ks,3,NULL);
    KeySysIFunc(ks,4,NULL);
    KeySysIFunc(ks,5,NULL);

    KeySysPrint(ks);
}
*/
