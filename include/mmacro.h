#ifndef __MMACRO_H__
#define __MMACRO_H__

#include <mcell.h>

#ifdef __cplusplus
extern "C"{
#endif

/* MMacro
 */
typedef struct __mmacro{
  MString key;
  MCell *arg;
  MCell *expr;
  struct __mmacro *next;
} MMacro;

void MMacroInit(MMacro *macro);
void MMacroDestroy(MMacro *macro);

void MMacroListInit(void);
void MMacroListAdd(MMacro *macro);
MMacro *MMacroCreateSimple(MString key, MString expr);
MMacro *MMacroCreate(MString key, MCell *arg, MCell *expr);

MMacro *MMacroFind(MString key);

MString MMacroExpand(MMacro *macro, MCell *arg, MString dest);
void MMacroClearArg(MMacro *macro);

#ifdef DEBUG
void MMacroPrint(MMacro *macro);
void MMacroListPrint(MMacro *macro);
#endif

void MMacroBuiltinInit(void);
void MMacroBuiltinDestroy(void);

#ifdef __cplusplus
}
#endif

#endif /* __MMACRO_H__ */
