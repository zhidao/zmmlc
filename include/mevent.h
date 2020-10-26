#ifndef __MEVENT_H__
#define __MEVENT_H__

#include <mpublic.h>

#ifdef __cplusplus
extern "C"{
#endif

/* message */

typedef struct{
  MBYTE size;
  MBYTE *array;
} MMSG;

void MMSGInit(MMSG *msg);
MMSG *MMSGCreate(MMSG *msg, MBYTE size);
void MMSGDestroy(MMSG *msg);

MMSG *MMSGVariableSizeVal(MMSG *msg, MInt val);

MMSG *MMSG2(MMSG *msg, MBYTE d0, MBYTE d1);
MMSG *MMSG3(MMSG *msg, MBYTE d0, MBYTE d1, MBYTE d2);
MMSG *MMSGMeta(MMSG *msg, MBYTE type, MBYTE size, MBYTE *data);
MMSG *MMSGExclusive(MMSG *msg, MBYTE size, MBYTE *data);

#ifdef DEBUG
void MMSGPrint(MMSG *msg);
#endif

/* atomic event */

typedef struct __mevent{
  MBYTE trk; /* track no. */
  MInt t;    /* time stamp */
  MMSG msg;  /* message body */
  struct __mevent *next, *prev;
} MEvent;

void MEventInit(MEvent *event);
MEvent *MEventCreate(MBYTE trk, MInt t);
void MEventDestroy(MEvent *event);

MEvent *MEventCreateMSG2(MBYTE trk, MInt t, MBYTE d0, MBYTE d1);
MEvent *MEventCreateMSG3(MBYTE trk, MInt t, MBYTE d0, MBYTE d1, MBYTE d2);
MEvent *MEventCreateMetaMSG(MBYTE trk, MInt t, MBYTE type, MBYTE size, MBYTE *data);
MEvent *MEventCreateExclusiveMSG(MBYTE trk, MInt t, MBYTE size, MBYTE *data);

void MEventInsert(MEvent *cur, MEvent *event);

#ifdef DEBUG
void MEventPrint(MEvent *event);
#endif

/* event list */

typedef struct{
  MEvent head; /* head of event list */
} MEventList;

/* time stamp offset */
#define MTSO(l) (l)->head.t

void MEventListInit(MEventList *list);
void MEventListDestroy(MEventList *list);
void MEventListInsert(MEventList *list, MEvent *event);
#define MEventListIsEmpty(l) ( (l)->head.next == &(l)->head )

void MEventListInsertMSG2(MEventList *list, MBYTE trk, MInt t, MBYTE ev, MBYTE ch, MBYTE d1);
void MEventListInsertMSG3(MEventList *list, MBYTE trk, MInt t, MBYTE ev, MBYTE ch, MBYTE d1, MBYTE d2);
void MEventListInsertMetaMSG(MEventList *list, MBYTE trk, MInt t, MBYTE type, MBYTE size, MBYTE *data);
void MEventListInsertExclusiveMSG(MEventList *list, MBYTE trk, MInt t, MBYTE size, MBYTE *data);

#ifdef DEBUG
void MEventListPrint(MEventList *list);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MEVENT_H__ */
