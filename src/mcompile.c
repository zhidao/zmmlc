#include <mcompile.h>

/* temporary track file */

MTrack track;

/* SMF */

FILE *mid;
MString midname = NULL;

static void MMIDSetName(MString name);
static void MMIDOpen(void);
static void MMIDReopen(void);
static void MMIDClose(void);

static void MMIDHeaderChunk(void);
static void MMIDTrackChunk(MTrack *trk);
static void MMIDMergeTrack(MTrack *trk);

void MMIDSetName(MString name)
{
  char filename[MFILENAMELEN];

  strcpy( filename, name );
  MFileNameAddSuffix( filename, ".mid" );
  midname = MStringAlloc( filename );
}

void MMIDOpen(void)
{
  mid = fopen( midname, "wb" );
}

void MMIDReopen(void)
{
  mid = fopen( midname, "a+b" );
}

void MMIDClose(void)
{
  fclose( mid );
}

void MMIDPutInt(MInt val, MBYTE size)
{
  if( size == 0 ) return;
  MMIDPutInt( val >> 8, (MBYTE)( size - 1 ) );
  fputc( val & 0xff, mid );
}

void MMIDPutStr(MBYTE *str, MBYTE size)
{
  if( size == 0 ) return;
  fputc( *str, mid );
  MMIDPutStr( str+1, (MBYTE)( size - 1 ) );
}

/* write header chunk */
void MMIDHeaderChunk(void)
{
  MMIDPutStr( (MBYTE *)"MThd", 4 );
  MMIDPutInt( 6, 4 );
  MMIDPutInt( format, 2 );
  MMIDPutInt( tracknum, 2 );
  MMIDPutInt( timebase, 2 );
  if( verbose )
    MECHOHEADER( format, tracknum, timebase );
}

/* write track chunk */
void MMIDTrackChunk(MTrack *trk)
{
  MMIDPutStr( (MBYTE *)"MTrk", 4 );
  MMIDPutInt( trk->size, 4 );
  if( verbose )
    MECHOTRACK( trk->size );
}

/* write track chunk */
void MMIDMergeTrack(MTrack *trk)
{
  register unsigned int i;
  MBYTE data;

  if( trackno == 0 ){
    MMIDOpen();
    MMIDHeaderChunk();
  } else
    MMIDReopen();
  MMIDTrackChunk( trk );
  if( verbose )
    MECHOMERGE();
  MTrackOpen( trk, "rb" );
  for( i=0; i<trk->size; i++ ){
    data = (MBYTE)fgetc( trk->fp );
    fputc( data, mid );
  }
  MTrackClose( trk );
  MMIDClose();
}

/* compile */

void MCompileErrorCheck(void)
{
  if( errnum == 0 && warnnum == 0 ) return;
  MMESSAGEWORD( MHEAD_MMLFILE, srcname );
  if( warnnum > 0 )
    MWARNCOUNT();
  if( errnum > 0 ){
    MERRORCOUNT();
    MABORT();
  }
}

void MCompileTrack(MTrack *trk, MString filename)
{
  MCell *cell;

  MOpenSrc( filename );
  do{
    MGetBuf();
    cell = MParseLine();
    if( cell ){
      if( MCellIsCMD( cell, MML_CHANNEL ) )
        MEvalCHANNEL( trk, cell );
      if( cell->id >= MML_META )
        MEvalMETA( trk, cell );
      MCellDestroy( cell );
    }
    MTrackOutput( trk, mintime );
  } while( !feof( src ) );
  while( !MEventListIsEmpty( &trk->elist ) )
    MTrackOutput( trk, maxtime++ );
  MCompileErrorCheck();
  MCloseSrc();
}

void MCompile(MString srcfile, MString midfile)
{
  char filename[MFILENAMELEN];

  MMIDSetName( midfile );
  strcpy( filename, midfile );
  MFileNameAddSuffix( filename, ".trk" );

  do{
    MMacroBuiltinInit();
    if( verbose )
      MCOMPILETRACK( trackno );
    MTrackInit( &track, filename );
    MTrackOpen( &track, "wb" );
    if( format == 1 )
      MTrackOutputPort( &track );
    MCompileTrack( &track, srcfile );
    if( !MEventListIsEmpty( &track.elist ) )
      MINNERERROR();
    MChannelDestroy();
    MTrackOutputEOT( &track );
    MTrackClose( &track );
    MMIDMergeTrack( &track );
    MTrackDestroy( &track );
    MMacroBuiltinDestroy();
  } while( ++trackno < tracknum );
  if( verbose )
    MECHOEND( MHEAD_FORMAT0 + format, midname );
  MFREE( midname );
}
