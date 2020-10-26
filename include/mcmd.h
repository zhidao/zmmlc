#ifndef __MCMD_H__
#define __MCMD_H__

#include <mcell.h>

#ifdef __cplusplus
extern "C"{
#endif

enum{
  MML_METACOM = 1,
  MML_VAR,
  MML_PAR,
  MML_CHANNEL,
  MML_COMMENT,
  MML_NOTE,
  MML_REST,
  MML_POLYKEYPRESS,
  MML_CONTROLCHANGE,
  MML_PROGRAMCHANGE,
  MML_CHANNELPRESS,
  MML_PITCHBEND,
  MML_MASTERVOLUME,
  MML_TEMPO,
  MML_TRANSPOSE,
  MML_BASENOTE,
  MML_DEFAULTLEN,
  MML_DEFAULTVEL,
  MML_GATETIME,
  MML_CHANGECHANNEL,
  MML_DIVIDECHANNEL,
  MML_EXCLUSIVE,
  MML_GSEXCLUSIVE,
  MML_MACROCALL,
  MML_SEQUENCE,
  MML_CHORD_BEGIN,
  MML_CHORD_END,
  MML_REPEAT_BEGIN,
  MML_REPEAT_CASE,
  MML_REPEAT_END,
  MML_ENV_BEGIN,
  MML_ENV_END,
  MML_OFF,
  MML_HOLD,
  MML_INCREMENT,
  MML_DECREMENT,
  MML_TIE,
  MML_SHARP,
  MML_FLAT,
  MML_HALF,
  MML_PRECISION,
  MML_HEXADECIMAL,
  MML_QUOTATION,
  MML_DELIMITER,
  MML_NL,
};

/* MCMD
 */
typedef struct{
  MString name;
  MBYTE id;
  char key;
  MCell *arg;
} MCMD;

char MCMDKey(int id);
int MCMDCmp(MBYTE id, char key);

MString MCMDName(MBYTE id);
MCMD *MCMDNameToCMD(MString name);
void MCMDSetKey(MCMD *cmd, char key);

int MCMDIsIndependent(char key);

MCMD *MCMDFind(MBYTE id);
MCMD *MCMDFindKey(char key);

#ifdef DEBUG
void MCMDPrint(MCMD *cmd);
void MCMDListPrint(void);
#endif

void MCMDBuiltinInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __MCMD_H__ */
