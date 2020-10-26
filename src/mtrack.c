#include <mtrack.h>
#include <mmidi.h>

/* temporary track file operation */

MBYTE trackno = 0;

static void MTrackOutputBYTE(MTrack *trk, MBYTE data);
static void MTrackOutputVariableSizeVal(MTrack *trk, MInt val, MBYTE flag);
static void MTrackOutputMessage(MTrack *trk, MMSG *msg);
static void MTrackOutputEvent(MTrack *trk, MEvent *event);

static void MTrackDestroyEvent(MTrack *trk, MEvent *ep);

static void MTrackOutput0(MTrack *trk, MInt time);
static int MTrackEventAcceptable(MEvent *event);
static void MTrackOutput1(MTrack *trk, MInt time);

void MTrackInit(MTrack *trk, MString filename)
{
  if( filename == NULL )
    MINNERERROR();
  trk->name = MStringAlloc( filename );
  trk->fp = NULL;
  trk->size = 0;
  trk->runningstatus = 0;

  MEventListInit( &trk->elist );
}

void MTrackDestroy(MTrack *trk)
{
  remove( trk->name );
  MFREE( trk->name );
  trk->fp = NULL;
  trk->size = 0;
  trk->runningstatus = 0;

  MEventListDestroy( &trk->elist );
}

void MTrackOpen(MTrack *trk, char *mode)
{
  trk->fp = fopen( trk->name, mode );
  if( trk->fp == NULL )
    MOPENERROR( trk->name );
}

void MTrackClose(MTrack *trk)
{
  fclose( trk->fp );
}

void MTrackOutputBYTE(MTrack *trk, MBYTE data)
{
  fputc( data, trk->fp );
  trk->size++;
}

void MTrackOutputVariableSizeVal(MTrack *trk, MInt val, MBYTE flag)
{
  if( val >= 0x80 )
    MTrackOutputVariableSizeVal( trk, val >> 7, 0x80 );
  MTrackOutputBYTE( trk, (MBYTE)( val & 0x7f ) | flag );
}

void MTrackOutputMessage(MTrack *trk, MMSG *msg)
{
  register int i = 0;

  /* running status */
  if( MBYTEIsStatus( *msg->array ) ){
    if( MBYTEStatus( *msg->array ) == 0xf )
      trk->runningstatus = 0x00; /* clear */
    else{
      if( *msg->array == trk->runningstatus )
        i = 1;
      else
        trk->runningstatus = *msg->array;
    }
  }
  for( ; i<msg->size; i++ )
    MTrackOutputBYTE( trk, *( msg->array + i ) );
}

void MTrackOutputEvent(MTrack *trk, MEvent *event)
{
  MInt dt;

  /* calculation of delta time */
  dt = event->t - MTSO( &trk->elist );
  /* time stamp offset update */
  MTSO( &trk->elist ) = event->t;
  /* output of delta time as a variable size value */
  MTrackOutputVariableSizeVal( trk, dt, 0x0 );
  /* output of message(event body) */
  MTrackOutputMessage( trk, &event->msg );
}

void MTrackOutputPort(MTrack *trk)
{
#define MPORTTRACKNUM 16
  MTrackOutputBYTE( trk, 0x00 );
  MTrackOutputBYTE( trk, 0xff );
  MTrackOutputBYTE( trk, 0x21 );
  MTrackOutputBYTE( trk, 0x01 );
  MTrackOutputBYTE( trk, (MBYTE)( trackno / MPORTTRACKNUM ) );
}

void MTrackOutputEOT(MTrack *trk)
{
  MTrackOutputBYTE( trk, 0x00 );
  MTrackOutputBYTE( trk, 0xff );
  MTrackOutputBYTE( trk, 0x2f );
  MTrackOutputBYTE( trk, 0x00 );
}

void MTrackDestroyEvent(MTrack *trk, MEvent *ep)
{
  trk->elist.head.next = ep->next;
  ep->next->prev = &trk->elist.head;
  MEventDestroy( ep );
}

/* format 0 case */
void MTrackOutput0(MTrack *trk, MInt time)
{
  MEvent *ep;

  /* events which will occur until 'time' are output
   * to the temporary track file */
  while( 1 ){
    ep = trk->elist.head.next;
    if( ep == &trk->elist.head || ep->t >= time ) break;
    MTrackOutputEvent( trk, ep );
    /* destruction of output atomic event */
    MTrackDestroyEvent( trk, ep );
  }
}

int MTrackEventAcceptable(MEvent *event)
{
  /* This function is only called from MTrackOutput1 */
  return event->trk == trackno;
}

/* format 1 case */
void MTrackOutput1(MTrack *trk, MInt time)
{
  MEvent *ep;

  /* events which will occur until 'time' are output
   * to the temporary track file */
  while( 1 ){
    ep = trk->elist.head.next;
    if( ep == &trk->elist.head || ep->t >= time ) break;
    if( MTrackEventAcceptable( ep ) )
      MTrackOutputEvent( trk, ep );
    /* destruction of output atomic event */
    MTrackDestroyEvent( trk, ep );
  }
}

void MTrackOutput(MTrack *trk, MInt time)
{
  switch( format ){
  case 0:  MTrackOutput0( trk, time ); break;
  case 1:  MTrackOutput1( trk, time ); break;
  default: MINNERERROR();
  }
}
