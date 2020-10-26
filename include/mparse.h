#ifndef __MPARSE_H__
#define __MPARSE_H__

#include <mcmd.h>
#include <mmeta.h>
#include <mmacro.h>

#ifdef __cplusplus
extern "C"{
#endif

extern FILE *src;
extern MString srcname;
extern MInt linenum;

extern char pbuf[], *bufcur;
extern char token[];

void MOpenSrc(MString name);
void MCloseSrc(void);
MString MGetBuf(void);

/* tokenization */

int MIsTerminator(char c);
int MIsWS(char c);

MString MSkipWS(void);
MString MGetToken(void);
MString MGetString(void);
MString MGetHex(void);
MInt MHexToInt(MString hex);

MCell *MGetINT(void);
MCell *MGetVAL(void);
MCell *MGetLEN(void);
MCell *MGetKEY(void);
MCell *MGetSTR(void);
MCell *MGetCMD(MCMD *cmd);
MCell *MGetCMDWithARG(MCMD *cmd);
MCell *MGetMETA(void);
MCell *MGetENV(void);
MCell *MGetINTLIST(void);
MCell *MGetSTRLIST(void);
MCell *MGetENVLIST(void);
MCell *MGetARG(MCell *list);

MCell *MStringToINTLIST(MString str);
void MGetMacro(MMacro *macro, MCell *arg);
MCell *MGetMacroArg(MMacro *macro);

/* parsing */

extern MBYTE mask;

MCell *MParseLine(void);
MCell *MParseCMD(void);
MCell *MParseChannel(void);

#ifdef __cplusplus
}
#endif

#endif /* __MPARSE_H__ */
