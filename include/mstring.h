#ifndef __MSTRING_H__
#define __MSTRING_H__

#include <mpublic.h>

#ifdef __cplusplus
extern "C"{
#endif

#include <string.h>

#define MBUFSIZ      512
#define MFILENAMELEN 128

#ifdef DEBUG
void MIndent(int indent);
#endif

typedef char *MString;

#define MStringClear(s) s[0] = '\0'

MString MStringCat(MString dest, MString src);
int MStringLen(MString str);
int MStringCmp(MString s1, MString s2);
MString MStringAlloc(MString str);

MString MStringI(MInt i);
MString MStringHex(MBYTE h);

int MFileNameHasSuffix(MString name, MString suffix);
void MFileNameAddSuffix(MString name, MString suffix);

#ifdef __cplusplus
}
#endif

#endif /* __MSTRING_H__ */
