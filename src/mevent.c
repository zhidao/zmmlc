#include <mevent.h>

/* MIDI event message */

void MMSGInit(MMSG *msg)
{
  msg->size = 0;
  msg->array = NULL;
}

MMSG *MMSGCreate(MMSG *msg, MBYTE size)
{
  msg->size = size;
  msg->array = MALLOC( MBYTE, size );
  return msg;
}

void MMSGDestroy(MMSG *msg)
{
  msg->size = 0;
  MFREE( msg->array );
}

#define MINTARRSIZ 6
MMSG *MMSGVariableSizeVal(MMSG *msg, MInt val)
{
  register int i;
  static MBYTE vbuf[MINTARRSIZ];
  MBYTE *bp, size=1;

  bp = vbuf + MINTARRSIZ - 1;
  *bp = (MBYTE)( val & 0x7f );
  val >>= 7;

  if( val >= 1 ){
    for( bp--; bp>=vbuf; bp--, size++ ){
      *bp = (MBYTE)( ( val & 0x7f ) | 0x80 );
      val >>= 7;
      if( val < 1 ) break;
    }
  }
  MMSGCreate( msg, size );
  for( i=0; i<size; i++ )
    msg->array[i] = *( bp + i );
  return msg;
}

MMSG *MMSG2(MMSG *msg, MBYTE d0, MBYTE d1)
{
  MMSGCreate( msg, 2 );
  msg->array[0] = d0;
  msg->array[1] = d1;
  return msg;
}

MMSG *MMSG3(MMSG *msg, MBYTE d0, MBYTE d1, MBYTE d2)
{
  MMSGCreate( msg, 3 );
  msg->array[0] = d0;
  msg->array[1] = d1;
  msg->array[2] = d2;
  return msg;
}

MMSG *MMSGMeta(MMSG *msg, MBYTE type, MBYTE size, MBYTE *data)
{
  register MBYTE i;
  MMSG sizemsg;

  MMSGVariableSizeVal( &sizemsg, size );
  MMSGCreate( msg, (MBYTE)( 2+sizemsg.size+size ) );
  msg->array[0] = 0xff;
  msg->array[1] = type;
  for( i=0; i<sizemsg.size; i++ )
    *( msg->array + 2 + i ) = sizemsg.array[i];
  for( i=0; i<size; i++ )
    *( msg->array + 2 + sizemsg.size + i ) = data[i];
  MMSGDestroy( &sizemsg );
  return msg;
}

MMSG *MMSGExclusive(MMSG *msg, MBYTE size, MBYTE *data)
{
  register MBYTE i;
  MMSG sizemsg;

  MMSGVariableSizeVal( &sizemsg, size );
  MMSGCreate( msg, (MBYTE)( 1+sizemsg.size+size ) );
  msg->array[0] = (MBYTE)0xf0;
  for( i=0; i<sizemsg.size; i++ )
    *( msg->array + 1 + i ) = sizemsg.array[i];
  for( i=0; i<size; i++ )
    *( msg->array + 1 + sizemsg.size + i ) = data[i];
  MMSGDestroy( &sizemsg );
  return msg;
}

#ifdef DEBUG
void MMSGPrint(MMSG *msg)
{
  register int i;

  printf( "msg:" );
  for( i=0; i<msg->size; i++ )
    MPrintHex( *( msg->array + i ) );
  printf( "\n" );
}
#endif

/* MIDI event list */

void MEventInit(MEvent *event)
{
  event->trk = 0;
  event->t = 0;
  MMSGInit( &event->msg );
  event->next = event->prev = event;
}

MEvent *MEventCreate(MBYTE trk, MInt t)
{
  MEvent *event;

  event = MALLOC( MEvent, 1 );
  if( event == NULL )
    MALLOCERROR();

  MEventInit( event );
  event->trk = trk;
  event->t = t;
  return event;
}

void MEventDestroy(MEvent *event)
{
  MMSGDestroy( &event->msg );
  MFREE( event );
}

MEvent *MEventCreateMSG2(MBYTE trk, MInt t, MBYTE d0, MBYTE d1)
{
  MEvent *event;

  event = MEventCreate( trk, t );
  MMSG2( &event->msg, d0, d1 );
  return event;
}

MEvent *MEventCreateMSG3(MBYTE trk, MInt t, MBYTE d0, MBYTE d1, MBYTE d2)
{
  MEvent *event;

  event = MEventCreate( trk, t );
  MMSG3( &event->msg, d0, d1, d2 );
  return event;
}

MEvent *MEventCreateMetaMSG(MBYTE trk, MInt t, MBYTE type, MBYTE size, MBYTE *data)
{
  MEvent *event;

  event = MEventCreate( trk, t );
  MMSGMeta( &event->msg, type, size, data );
  return event;
}

MEvent *MEventCreateExclusiveMSG(MBYTE trk, MInt t, MBYTE size, MBYTE *data)
{
  MEvent *event;

  event = MEventCreate( trk, t );
  MMSGExclusive( &event->msg, size, data );
  return event;
}

void MEventInsert(MEvent *cur, MEvent *event)
{
  event->next = cur->next;
  event->prev = cur;
  cur->next->prev = event;
  cur->next = event;
}

#ifdef DEBUG
void MEventPrint(MEvent *event)
{
  printf( "[event]\n" );
  printf( "track: %u\n", event->trk );
  printf( "time stamp: %u\n", event->t );
  MMSGPrint( &event->msg );
}
#endif

void MEventListInit(MEventList *list)
{
  MEventInit( &list->head );
}

void MEventListDestroy(MEventList *list)
{
  MEvent *ep;

  while( !MEventListIsEmpty( list ) ){
    ep = list->head.next;
    list->head.next = ep->next;
    ep->next->prev = &list->head;
    MEventDestroy( ep );
  }
  MEventListInit( list );
}

void MEventListInsert(MEventList *list, MEvent *event)
{
  MEvent *ep;

  for( ep=list->head.prev; ep!=&list->head; ep=ep->prev )
    if( ep->t <= event->t ) break;
  MEventInsert( ep, event );
}

void MEventListInsertMSG2(MEventList *list, MBYTE trk, MInt t, MBYTE ev, MBYTE ch, MBYTE d1)
{
  if( ch > 0xf )
    MERROR( MERROR_CHANNEL );
  else
    MEventListInsert( list, MEventCreateMSG2( trk, t, ev | ch, d1 ) );
}

void MEventListInsertMSG3(MEventList *list, MBYTE trk, MInt t, MBYTE ev, MBYTE ch, MBYTE d1, MBYTE d2)
{
  if( ch > 0xf )
    MERROR( MERROR_CHANNEL );
  else
    MEventListInsert( list, MEventCreateMSG3( trk, t, ev | ch, d1, d2 ) );
}

void MEventListInsertMetaMSG(MEventList *list, MBYTE trk, MInt t, MBYTE type, MBYTE size, MBYTE *data)
{
  MEventListInsert( list, MEventCreateMetaMSG( trk, t, type, size, data ) );
}

void MEventListInsertExclusiveMSG(MEventList *list, MBYTE trk, MInt t, MBYTE size, MBYTE *data)
{
  MEventListInsert( list, MEventCreateExclusiveMSG( trk, t, size, data ) );
}

#ifdef DEBUG
void MEventListPrint(MEventList *list)
{
  MEvent *ep;

  if( MEventListIsEmpty( list ) )
    printf( "[no event]\n" );
  else
    for( ep=list->head.next; ep!=&list->head; ep=ep->next )
      MEventPrint( list->head.next );
}
#endif
