#ifndef __MMIDI_H__
#define __MMIDI_H__

#include <mpublic.h>

#ifdef __cplusplus
extern "C"{
#endif

extern MBYTE format;
extern MInt timebase;

extern MBYTE tracknum;
extern MBYTE channelnum;
extern MInt maxtime, mintime;

void MFormatCheck(void);
void MTimebaseCheck(void);

/* channel */

typedef struct{
  MInt  len;       /* default note length */
  signed char oct; /* octave */
  signed char tr;  /* transpose */
  MBYTE gt;        /* gatetime numerator */
  MBYTE gtd;       /* gatetime denominator */
} MChannelParam;

typedef struct{
  MBYTE id;        /* channel ID */
  MBYTE ch;        /* real channel ID */
  MInt  time;      /* current time stamp */
  MChannelParam p; /* channel parameter set */
  MBYTE vel;       /* velocity */
} MChannel;

extern MChannel *channel, *curch;

void MChannelInit(MChannel *ch, MBYTE id);
void MChannelCreate(MBYTE n);
void MChannelDestroy(void);
MChannel *MChannelFind(MBYTE id);

void MChannelTimeUpdate(MChannel *ch, MInt t);
void MChannelTimeIncrement(MChannel *ch, MInt dt);
MInt MChannelMinTime(void);

#ifdef DEBUG
void MChannelPrint(MChannel *ch);
void MChannelPrintAll(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MMIDI_H__ */
