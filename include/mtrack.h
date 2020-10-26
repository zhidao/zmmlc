#ifndef __MTRACK_H__
#define __MTRACK_H__

#include <mstring.h>
#include <mmidi.h>
#include <mevent.h>

#ifdef __cplusplus
extern "C"{
#endif

/* temporary track file operation */

typedef struct{
  MString name;        /* temporary file name */
  FILE *fp;            /* temporary file pointer */
  MInt  size;          /* track byte size */
  MBYTE runningstatus; /* running status */
  MEventList elist;    /* event list */
} MTrack;

void MTrackInit(MTrack *trk, MString name);
void MTrackDestroy(MTrack *trk);
void MTrackOpen(MTrack *trk, char *mode);
void MTrackClose(MTrack *trk);

void MTrackOutput(MTrack *trk, MInt time);
void MTrackOutputPort(MTrack *trk);
void MTrackOutputEOT(MTrack *trk);

extern MBYTE trackno;

#ifdef __cplusplus
}
#endif

#endif /* __MTRACK_H__ */
