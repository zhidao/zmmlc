#ifndef __MMETA_H__
#define __MMETA_H__

#include <mcell.h>

#ifdef __cplusplus
extern "C"{
#endif

#define MML_META_SYNC        1
#define MML_META_ECHO        2
#define MML_META_DEFMACRO    3
#define MML_META_DEFCMD      4
#define MML_META_MASK        5
#define MML_META_UNMASK      6
#define MML_META_REPEAT      7
#define MML_META_REPEATCASE  8
#define MML_META_REPEATEND   9
#define MML_META_FINE       10
#define MML_META_TOCODA     11
#define MML_META_CODA       12
#define MML_META_INCLUDE    13
#define MML_META_MARKER     14
#define MML_META_CUE        15
#define MML_META_TEXT       16
#define MML_META_LYRIC      17
#define MML_META_TRACK      18
#define MML_META_INSTRUMENT 19
#define MML_META_BEAT       20
#define MML_META_CODE       21
#define MML_META_FORMAT     22
#define MML_META_TIMEBASE   23
#define MML_META_PLAYLIST   24
#define MML_META_UNPLAYLIST 25
#define MML_META_COPYRIGHT  26
#define MML_META_TITLE      27

typedef struct{
  MString name;
  MBYTE id;
  MCell *arg;
} MMeta;

void MMetaInit(void);
MString MMetaName(MBYTE id);
MMeta *MMetaFind(MString key);

#ifdef DEBUG
void MMetaPrint(MMeta *meta);
void MMetaListPrint(void);
#endif

/* code name */

int MCodeFind(MString name, MBYTE val[]);

#ifdef __cplusplus
}
#endif

#endif /* __MMETA_H__ */
