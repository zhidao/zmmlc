#ifndef __MPUBLIC_H__
#define __MPUBLIC_H__

#ifdef __cplusplus
extern "C"{
#endif

#define ZMMLC_VERSION "3.3"

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char MBYTE;

#define MBYTEIsStatus( b ) ( ( (b) & 0x80 ) >> 7 )
#define MBYTEStatus( b )   ( (b) >> 4 )
#define MBYTEChannel( b )  ( (b) & 0xf )

#ifdef __16BIT__
typedef unsigned long MInt;
#else
typedef unsigned int  MInt;
#endif

#define MASCII2ID(c) ( (c) - '!' )
#define MASCII_NUM   ( MASCII2ID( '~' ) + 1 )

/* error message header ID */
enum{
  MHEAD_VERSION = 0,
  MHEAD_ERROR,
  MHEAD_WARN,
  MHEAD_TRACK,
  MHEAD_COMPILE,
  MHEAD_MMLFILE,
  MHEAD_OPEN1,
  MHEAD_OPEN2,
  MHEAD_FORMAT,
  MHEAD_TRACKNUM,
  MHEAD_TIMEBASE,
  MHEAD_TRACKSIZE,
  MHEAD_BYTES,
  MHEAD_MERGE,
  MHEAD_FORMAT0,
  MHEAD_FORMAT1,
  MHEAD_OUTPUT,
  MHEAD_MIDFILE,
  MHEAD_ABORT,
  MHEAD_USAGE,
};

/* error message body ID */
enum{
  /* mpublic */
  MERROR_OPENFILE = 0,
  MERROR_MEMORYALLOC,
  /* mparse */
  MERROR_MISSINGCHO,
  MERROR_MISSINGREP,
  MERROR_MISSINGENV,
  MERROR_MISMATCHCHO,
  MERROR_REPCASE,
  MERROR_MISMATCHREP,
  MERROR_UNKNOWNMETA,
  MERROR_UNKNOWNCMD,
  MERROR_UNKNOWNMACRO,
  /* meval */
  MERROR_NOTENO,
  MERROR_NOTEOUTOFRAN,
  MERROR_NEGATIVELEN,
  MERROR_VELOUTOFRAN,
  MERROR_CTRLNO,
  MERROR_VALOUTOFRAN,
  MERROR_CURVE,
  MERROR_ZEROGTD,
  MERROR_SEQ,
  MERROR_FORMAT2,
  MERROR_FORMAT,
  MERROR_BEAT,
  MERROR_CODE,
  MERROR_CHANNEL,
  MERROR_CHANNELNOTFOUND,
  MERROR_TRACK,
  MERROR_DEFCMD,
  MERROR_DEFKEY,
  MERROR_DEFUNKNOWN,
  MERROR_DEFMACRO,
  MERROR_MACROVAR,
  MERROR_ARG,
  MERROR_REPTIME,
  MERROR_TIMEBASE,
  MERROR_OVERWRITE,
  MERROR_MASK,
  MERROR_METAREPEAT,
  MERROR_CODA,
  /* mcompile */
  MERROR_INNER,
  MERROR_APPOLOGY,
  MERROR_WARNING,
  MERROR_ERROR,
  /* command line */
  MERROR_OUTPUTFILE,
  MERROR_UNKNOWNOPTION,
  MERROR_SRCFILE,
};

/* switch for verbose mode */
extern MBYTE verbose;

/* error & warning counter */
extern int errnum, warnnum;

/* error message output functions */
void MLINE(int linenum);

void MECHO(char *msg);
void MMESSAGE(int id);
void MMESSAGEWORD(int id, char *wrd);
void MERROR(int id);
void MERRORWORD(int id, char *wrd);
void MWARN(int id);
void MWARNWORD(int id, char *wrd);
void MOPENERROR(char *filename);
void MALLOCERROR(void);
void MINNERERROR(void);
void MERRORCOUNT(void);
void MWARNCOUNT(void);
void MCOMPILETRACK(int n);
void MECHOOPENMML(char *filename);
void MECHOHEADER(int format, int tracknum, int timebase);
void MECHOTRACK(int tracksize);
void MECHOMERGE(void);
void MECHOEND(int format, char *filename);

void MLINEERROR(int line, int id, char *wrd);
void MLINEWARN(int line, int id, char *wrd);

/* memory management */
#define MALLOC(t,s)    (t *)malloc( sizeof(t)*(s) )
#define MFREE(m)       if((m)){ free(m); }
#define MFREE_RESET(m) if((m)){ free(m); (m) = NULL; }

#ifdef DEBUG
/* hexadecimal number output */
# define MPrintHex( h ) \
  printf( " %1X%1X", ((h)>>4), (h)&0xf )
#endif

#define MML_INVALID 0

void MABORT(void);
void MUSAGE(void);
void MZMMLC(void);

#ifdef __cplusplus
}
#endif

#endif /* __MPUBLIC_H__ */
