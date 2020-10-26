#ifndef __MEVAL_H__
#define __MEVAL_H__

#include <mparse.h>
#include <mtrack.h>

#ifdef __cplusplus
extern "C"{
#endif

/* cell evaluation */

void MEvalCMD(MTrack *trk, MCell *cell);
void MEvalMETA(MTrack *trk, MCell *cell);

/* CMD evaluation */

void MEvalCHANNEL(MTrack *trk, MCell *cell);
void MEvalNOTE(MTrack *trk, MCell *cell);
void MEvalREST(MTrack *trk, MCell *cell);
void MEvalPOLYKEYPRESS(MTrack *trk, MCell *cell);
void MEvalCONTROLCHANGE(MTrack *trk, MCell *cell);
void MEvalPROGRAMCHANGE(MTrack *trk, MCell *cell);
void MEvalCHANNELPRESS(MTrack *trk, MCell *cell);
void MEvalPITCHBEND(MTrack *trk, MCell *cell);
void MEvalMASTERVOLUME(MTrack *trk, MCell *cell);
void MEvalTEMPO(MTrack *trk, MCell *cell);
void MEvalTRANSPOSE(MTrack *trk, MCell *cell);
void MEvalBASENOTE(MTrack *trk, MCell *cell);
void MEvalDEFAULTLEN(MTrack *trk, MCell *cell);
void MEvalDEFAULTVEL(MTrack *trk, MCell *cell);
void MEvalGATETIME(MTrack *trk, MCell *cell);
void MEvalCHANGECHANNEL(MTrack *trk, MCell *cell);
void MEvalDIVIDECHANNEL(MTrack *trk, MCell *cell);
void MEvalEXCLUSIVE(MTrack *trk, MCell *cell);
void MEvalGSEXCLUSIVE(MTrack *trk, MCell *cell);
void MEvalSEQUENCE(MTrack *trk, MCell *cell);
void MEvalCHORD(MTrack *trk, MCell *cell);
void MEvalREPEAT(MTrack *trk, MCell *cell);

/* meta command evaluation */

void MEvalTEXT(MTrack *trk, MCell *cell);
void MEvalCOPYRIGHT(MTrack *trk, MCell *cell);
void MEvalTITLE(MTrack *trk, MCell *cell);
void MEvalTRACK(MTrack *trk, MCell *cell);
void MEvalINSTRUMENT(MTrack *trk, MCell *cell);
void MEvalLYRIC(MTrack *trk, MCell *cell);
void MEvalMARKER(MTrack *trk, MCell *cell);
void MEvalCUE(MTrack *trk, MCell *cell);
void MEvalBEAT(MTrack *trk, MCell *cell);
void MEvalCODE(MTrack *trk, MCell *cell);
void MEvalFORMAT(MTrack *trk, MCell *cell);
void MEvalTIMEBASE(MTrack *trk, MCell *cell);
void MEvalINCLUDE(MTrack *trk, MCell *cell);
void MEvalPLAYLIST(MTrack *trk, MCell *cell);
void MEvalUNPLAYLIST(MTrack *trk, MCell *cell);
void MEvalDEFCMD(MTrack *trk, MCell *cell);
void MEvalDEFMACRO(MTrack *trk, MCell *cell);
void MEvalMASK(MTrack *trk, MCell *cell);
void MEvalUNMASK(MTrack *trk, MCell *cell);
void MEvalMETAREPEAT(MTrack *trk, MCell *cell);
void MEvalMETAREPEATCASE(MTrack *trk, MCell *cell);
void MEvalMETAREPEATEND(MTrack *trk, MCell *cell);
void MEvalFINE(MTrack *trk, MCell *cell);
void MEvalTOCODA(MTrack *trk, MCell *cell);
void MEvalCODA(MTrack *trk, MCell *cell);
void MEvalECHO(MTrack *trk, MCell *cell);
void MEvalSYNC(MTrack *trk, MCell *cell);

#ifdef __cplusplus
}
#endif

#endif /* __MEVAL_H__ */
