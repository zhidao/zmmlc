#ifndef __MCELL_H__
#define __MCELL_H__

#include <mstring.h>

#ifdef __cplusplus
extern "C"{
#endif

#define MML_INT      1
#define MML_INT2     2
#define MML_VAL      3
#define MML_LEN      4
#define MML_KEY      5
#define MML_STR      6
#define MML_ENV      7
#define MML_INTLIST  8
#define MML_STRLIST  9
#define MML_ENVLIST 10
#define MML_PTR     11
#define MML_CMD     12
#define MML_META    58

/* MCell
 */
typedef struct __mcell{
  MBYTE id;
  union{
    MString expr;
    struct __mcell *cell;
    MInt intval;
  } val;
  struct __mcell *next;
} MCell;

void MCellInit(MCell *cell, MBYTE id);
void MCellDestroy(MCell *cell);

MCell *MCellCreate(MBYTE id);
MCell *MCellCreateSTR(MString str);
MCell *MCellCreateINT(MInt val);
MCell *MCellCreateINT2(MInt val);
MCell *MCellCreatePTR(MCell *arg);

#define MCellCreateCMD(i)  MCellCreate( (MBYTE)( MML_CMD + (i) ) )
#define MCellCreateMETA(i) MCellCreate( (MBYTE)( MML_META + (i) ) )

MCell *MCellCopy(MCell *org);

#define MCellIsCMD(c,i) ( (c)->id == MML_CMD+i )

MBYTE MCellListCount(MCell *list);
MCell *MCellListAdd(MCell *list, MCell *cell);

#ifdef DEBUG
void MCellPrint(MCell *cell, int indent);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MCELL_H__ */
