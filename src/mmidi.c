#include <mmidi.h>

#define MFORMAT_DEFAULT 0
#define MTIMEBASE_DEFAULT 0x60

MBYTE format = MFORMAT_DEFAULT;
MInt timebase = MTIMEBASE_DEFAULT;

MBYTE tracknum = 1;
MBYTE channelnum = 0;
MInt maxtime = 0, mintime = 0;

MChannel *channel = NULL, *curch = NULL;

void MFormatCheck(void)
{
  switch( format ){
  case 0:
    tracknum = 1;
    break;
  case 1:
    tracknum = 16;
    break;
  case 2:
    MERROR( MERROR_FORMAT2 );
    MABORT();
  default:
    MWARN( MERROR_FORMAT );
    format = MFORMAT_DEFAULT;
  }
}

void MTimebaseCheck(void)
{
  switch( timebase ){
  case 24: case 48: case 60: case 96: case 120:
  case 192: case 240: case 384: case 480: case 960:
    break;
  default:
    MWARN( MERROR_TIMEBASE );
    timebase = MTIMEBASE_DEFAULT;
  }
}

void MChannelInit(MChannel *ch, MBYTE id)
{
  ch->id   = ch->ch = id;
  ch->time = 0;
  ch->p.len  = timebase;
  ch->p.oct  = 4;
  ch->p.tr   = 0;
  ch->p.gt   = 8;
  ch->p.gtd  = 8;
  ch->vel    = 120;
}

void MChannelCreate(MBYTE n)
{
  register MBYTE i;

  channelnum = n;
  channel = MALLOC( MChannel, n );
  if( channel == NULL )
    MALLOCERROR();

  for( i=0; i<n; i++ )
    MChannelInit( &channel[i], i );
}

void MChannelDestroy(void)
{
  MFREE( channel );
  channelnum = 0;
  maxtime = mintime = 0;
}

MChannel *MChannelFind(MBYTE id)
{
  register int i;

  /* note that id=1...16 but actual channel id=0...15 */
  /* so, this function finds the channel 'id-1' */
  for( id--, i=0; i<channelnum; i++ )
    if( channel[i].id == id ) return &channel[i];
  return NULL;
}

void MChannelTimeUpdate(MChannel *ch, MInt t)
{
  ch->time = t;
  if( ch->time > maxtime )
    maxtime = ch->time;
  mintime = MChannelMinTime();
}

void MChannelTimeIncrement(MChannel *ch, MInt dt)
{
  MChannelTimeUpdate( ch, ch->time + dt );
}

MInt MChannelMinTime(void)
{
  register int i;
  MInt time;

  for( time=maxtime, i=0; i<channelnum; i++ )
    if( channel[i].time < time ) time = channel[i].time;
  return time;
}

#ifdef DEBUG
void MChannelPrint(MChannel *ch)
{
  printf( "channel[%d:%d]\n", ch->id, ch->ch );
  printf( "time stamp       = %d\n", ch->time );
  printf( "default length   = %d\n", ch->p.len );
  printf( "octave           = %d\n", ch->p.oct );
  printf( "transpose        = %d\n", ch->p.tr );
  printf( "gatetime         = %d / %d\n", ch->p.gt, ch->p.gtd );
  printf( "default velocity = %d\n", ch->vel );
}

void MChannelPrintAll(void)
{
  register int i;

  for( i=0; i<channelnum; i++ )
    MChannelPrint( &channel[i] );
}
#endif
