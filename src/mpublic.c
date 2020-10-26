#include <mpublic.h>

/* locale & charactor set detection */
#define EUC_JP    0
#define SHIFT_JIS 1
#define ENGLISH   2

/* switch for verbose mode */
MBYTE verbose = 0;

/* error message buffer */
#define MERRORMSGSIZ 512
static char merrormsg[MERRORMSGSIZ];
#ifndef __GUI__
/* simple error message output to the standerd error */
void MERROROUT(void)
{
  fprintf( stderr, "%s\n", merrormsg );
}
#else
/* dummy for GUI mode */
void MERROROUT(void)
{
  /* dummy */
}
#endif

/* error message catalogue
 * (choose one according to the locale.)
 */
#if ( LANG == EN )
#include "merror.en.mes"
#elif ( LANG == EUC_JP )
#include "merror.euc.mes"
#elif ( LANG == SHIFT_JIS )
#include "merror.sjis.mes"
#else
#include "merror.UTF-8.mes"
#endif

/* error & warning counter */
int errnum = 0, warnnum = 0;

void MECHO(char *msg)
{
  sprintf( merrormsg, "%s.", msg );
  MERROROUT();
}

void MMESSAGE(int id)
{
  MECHO( msghd[id] );
}

void MMESSAGEWORD(int id, char *wrd)
{
  sprintf( merrormsg, "%s %s.", msghd[id], wrd );
  MERROROUT();
}

void MERROR(int id)
{
  errnum++;
  sprintf( merrormsg, "%s: %s.", msghd[MHEAD_ERROR], errmsg[id] );
  MERROROUT();
}

void MERRORWORD(int id, char *wrd)
{
  errnum++;
  sprintf( merrormsg, "%s: %s <- %s.",
    msghd[MHEAD_ERROR], wrd, errmsg[id] );
  MERROROUT();
}

void MWARN(int id)
{
  warnnum++;
  sprintf( merrormsg, "%s: %s.", msghd[MHEAD_WARN], errmsg[id] );
  MERROROUT();
}

void MWARNWORD(int id, char *wrd)
{
  warnnum++;
  sprintf( merrormsg, "%s: %s <- %s.",
    msghd[MHEAD_WARN], wrd, errmsg[id] );
  MERROROUT();
}

void MOPENERROR(char *filename)
{
  MERRORWORD( MERROR_OPENFILE, filename ); MABORT();
}

void MALLOCERROR(void)
{
  MERROR( MERROR_MEMORYALLOC ); MABORT();
}

void MINNERERROR(void)
{
  MERROR( MERROR_INNER );
  MERROR( MERROR_APPOLOGY );
  MABORT();
}

void MERRORCOUNT(void)
{
  sprintf( merrormsg, "%d %s.", errnum, errmsg[MERROR_ERROR] );
  MERROROUT();
}

void MWARNCOUNT(void)
{
  sprintf( merrormsg, "%d %s.", warnnum, errmsg[MERROR_WARNING] );
  MERROROUT();
}

void MCOMPILETRACK(int n)
{
  sprintf( merrormsg, "%s %d %s",
    msghd[MHEAD_TRACK], n, msghd[MHEAD_COMPILE] );
  MERROROUT();
}

void MECHOOPENMML(char *filename)
{
  sprintf( merrormsg, "%s %s %s.",
    msghd[MHEAD_OPEN1], filename, msghd[MHEAD_OPEN2] );
  MERROROUT();
}

void MECHOHEADER(int format, int tracknum, int timebase)
{
  sprintf( merrormsg, "%s %d", msghd[MHEAD_FORMAT], format );
  MERROROUT();
  sprintf( merrormsg, "%s %d", msghd[MHEAD_TRACKNUM], tracknum );
  MERROROUT();
  sprintf( merrormsg, "%s %d", msghd[MHEAD_TIMEBASE], timebase );
  MERROROUT();
}

void MECHOTRACK(int tracksize)
{
  sprintf( merrormsg, "%s %d %s",
    msghd[MHEAD_TRACKSIZE], tracksize, msghd[MHEAD_BYTES] );
  MERROROUT();
}

void MECHOMERGE(void)
{
  sprintf( merrormsg, "%s.", msghd[MHEAD_MERGE] );
  MERROROUT();
}

void MECHOEND(int id, char *filename)
{
  MMESSAGE( id );
  sprintf( merrormsg, "%s %s %s.",
    msghd[MHEAD_OUTPUT], filename, msghd[MHEAD_MIDFILE] );
  MERROROUT();
}

void MLINEERROR(int line, int id, char *wrd)
{
  MLINE( line );
  if( wrd == NULL )
    MERROR( id );
  else
    MERRORWORD( id, wrd );
}

void MLINEWARN(int line, int id, char *wrd)
{
  MLINE( line );
  if( wrd == NULL )
    MWARN( id );
  else
    MWARNWORD( id, wrd );
}

#include <mtrack.h>
extern MTrack track;
void MABORT(void)
{
  MMESSAGE( MHEAD_ABORT );
  /* unlinking of trk files */
  MTrackDestroy( &track );
  exit( 1 );
}

void MUSAGE(void)
{
  strcpy( merrormsg, msghd[MHEAD_USAGE] );   MERROROUT();
  strcpy( merrormsg, msghd[MHEAD_USAGE+1] ); MERROROUT();
  strcpy( merrormsg, msghd[MHEAD_USAGE+2] ); MERROROUT();
  strcpy( merrormsg, msghd[MHEAD_USAGE+3] ); MERROROUT();
  strcpy( merrormsg, msghd[MHEAD_USAGE+4] ); MERROROUT();
  strcpy( merrormsg, msghd[MHEAD_USAGE+5] ); MERROROUT();
  strcpy( merrormsg, msghd[MHEAD_USAGE+6] ); MERROROUT();
  exit( 1 );
}

void MZMMLC(void)
{
  sprintf( merrormsg, "%s %s", msghd[MHEAD_VERSION], ZMMLC_VERSION );
  MERROROUT();
}
