#ifndef __MCOMPILE_H__
#define __MCOMPILE_H__

#include <mparse.h>
#include <meval.h>

#ifdef __cplusplus
extern "C"{
#endif

/* SMF */

extern FILE *mid;
extern MString midname;

/* compile */

void MCompileTrack(MTrack *trk, MString filename);
void MCompile(MString srcfile, MString midfile);

#ifdef __cplusplus
}
#endif

#endif /* __MCOMPILE_H__ */
