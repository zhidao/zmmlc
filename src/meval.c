#include <meval.h>
#include <mcompile.h>

static void (* evalcmd[])(MTrack *, MCell *) = {
  NULL, NULL, NULL, NULL, NULL, NULL,
  MEvalNOTE, MEvalREST,
  MEvalPOLYKEYPRESS, MEvalCONTROLCHANGE, MEvalPROGRAMCHANGE,
  MEvalCHANNELPRESS, MEvalPITCHBEND, MEvalMASTERVOLUME,
  MEvalTEMPO,
  MEvalTRANSPOSE, MEvalBASENOTE, MEvalDEFAULTLEN, MEvalDEFAULTVEL,
  MEvalGATETIME, MEvalCHANGECHANNEL,
  MEvalDIVIDECHANNEL, MEvalEXCLUSIVE, MEvalGSEXCLUSIVE,
  NULL, MEvalSEQUENCE,
  MEvalCHORD, NULL,
  MEvalREPEAT,
  NULL
};

static void (* evalmeta[])(MTrack *, MCell *) = {
  NULL,
  MEvalSYNC,
  MEvalECHO,
  MEvalDEFMACRO,
  MEvalDEFCMD,
  MEvalMASK,
  MEvalUNMASK,
  MEvalMETAREPEAT,
  MEvalMETAREPEATCASE,
  MEvalMETAREPEATEND,
  MEvalFINE,
  MEvalTOCODA,
  MEvalCODA,
  MEvalINCLUDE,
  MEvalMARKER,
  MEvalCUE,
  MEvalTEXT,
  MEvalLYRIC,
  MEvalTRACK,
  MEvalINSTRUMENT,
  MEvalBEAT,
  MEvalCODE,
  MEvalFORMAT,
  MEvalTIMEBASE,
  MEvalPLAYLIST,
  MEvalUNPLAYLIST,
  MEvalCOPYRIGHT,
  MEvalTITLE,
};

/* repeat stack */

enum{
  MML_REPSTACK_REPEAT,
  MML_REPSTACK_MASK,
  MML_REPSTACK_CODA,
};

typedef struct __mrepeatstack{
  unsigned long seek_offset;
  unsigned long lineno;
  MBYTE level;
  MBYTE count, maxcount;
  MBYTE mask;
  MBYTE type;
  struct __mrepeatstack *next;
} MRepeatStack;
static MRepeatStack repeatstack = { 0, 0, 0, 0, 0, 0, 0, NULL };

MBYTE MRepeatStackLevel(void)
{
  return repeatstack.next ? repeatstack.next->level : 0;
}

MBYTE MRepeatStackMask(void)
{
  return repeatstack.next ? repeatstack.next->mask : 0;
}

void MRepeatStackPush(MBYTE maxcount, MBYTE type, MBYTE mask)
{
  MRepeatStack *stack;

  stack = MALLOC( MRepeatStack, 1 );
  if( stack == NULL )
    MALLOCERROR();

  stack->seek_offset = ftell( src );
  stack->lineno = linenum;
  stack->level = MRepeatStackLevel();
  stack->count = 1;
  stack->maxcount = maxcount;
  stack->mask = mask;
  stack->type = type;
  stack->next = repeatstack.next;
  repeatstack.next = stack;
}

void MRepeatStackPop(void)
{
  MRepeatStack *stack;

  stack = repeatstack.next;
  if( stack ){
    repeatstack.next = stack->next;
    free( stack );
  } else{
    MINNERERROR();
    repeatstack.next = NULL;
  }
  mask = MRepeatStackMask();
}

#define MRepeatStackIsEmpty() ( repeatstack.next == NULL )

/* cell evaluation */

int MEvalINT(MCell *cell, int defaultval)
{
  return cell ? (int)cell->val.intval : defaultval;
}

int MEvalVAL(MCell *cell, int defaultval)
{
  if( cell == NULL ) return defaultval;
  if( MCellIsCMD( cell, MML_INCREMENT ) )
    return defaultval + MEvalINT( cell->next, defaultval );
  if( MCellIsCMD( cell, MML_DECREMENT ) )
    return defaultval - MEvalINT( cell->next, defaultval );
  return MEvalINT( cell, defaultval );
}

#define MEvalBYTE(c,v) (MBYTE)MEvalVAL( (c), (v) )

int MEvalLEN(MCell *cell)
{
#define MLEN(l)    ( (l) != 0 ? 4 * timebase / (l) : 0 )
  int l = 0, pl;
  MCell *prev = NULL;

  for( pl=curch->p.len; ; prev=cell, cell=cell->next ){
    if( cell == NULL ){
      if( prev == NULL || MCellIsCMD( prev, MML_TIE ) )
        l += curch->p.len;
      break;
    } else if( MCellIsCMD( cell, MML_TIE ) ){
      pl = curch->p.len;
      if( prev == NULL || MCellIsCMD( prev, MML_TIE ) )
        l += curch->p.len;
    } else if( MCellIsCMD( cell, MML_HALF ) ){
      pl /= 2;
      l += pl;
      if( prev == NULL || MCellIsCMD( prev, MML_TIE ) )
        l += curch->p.len;
    } else if( cell->id == MML_VAL ){
      pl = MLEN( MEvalVAL( cell->val.cell, 0 ) );
      l += pl;
    }
  }
  return l;
}

int MEvalKEY(MCell *cell)
{
  int key;

  for( key=0; cell!=NULL; cell=cell->next ){
    if( MCellIsCMD( cell, MML_SHARP ) )
      key++;
    else if( MCellIsCMD( cell, MML_FLAT ) )
      key--;
  }
  return key;
}

void MEvalCMD(MTrack *trk, MCell *cell)
{
  void (* f)(MTrack *, MCell *);

  f = evalcmd[ cell->id - MML_CMD ];
  if( f ) f( trk, cell );
}

void MEvalCMDLIST(MTrack *trk, MCell *cell)
{
  for( ; cell; cell=cell->next )
    MEvalCMD( trk, cell );
}

void MEvalMETA(MTrack *trk, MCell *cell)
{
  void (* f)(MTrack *, MCell *);

  f = evalmeta[ cell->id - MML_META ];
  if( f ) f( trk, cell );
}

/* cmd evaluation */

void MEvalCHANNEL(MTrack *trk, MCell *cell)
{
  MCell *ch;

  for( ch=cell->val.cell; ch; ch=ch->next ){
    curch = MChannelFind( (MBYTE)MEvalINT( ch, 0 ) );
    if( curch == NULL ) continue;
    MEvalCMDLIST( trk, cell->next );
  }
}

MBYTE MEvalNOTENO(MCell *cell)
{
  MBYTE note;

  if( cell->val.cell == NULL ){
    MLINEERROR( linenum, MERROR_NOTENO, NULL );
    return 0;
  }
  note = (MBYTE)( ( curch->p.oct + 1 ) * 12 );
  note = (MBYTE)( MEvalVAL( cell->val.cell, note ) + curch->p.tr );
  note += (MBYTE)MEvalKEY( cell->next->val.cell );
  if( note > 0x7f ){
    MLINEERROR( linenum, MERROR_NOTEOUTOFRAN, NULL );
    return 0;
  }
  return note;
}

static int notelennum = 1, notelenden = 1;

int MEvalNOTELEN(MCell *cell)
{
  int len;

  len = MEvalLEN( cell->val.cell );
  if( len < 0 ){
    MLINEERROR( linenum, MERROR_NEGATIVELEN, NULL );
    return 0;
  }
  return len * notelennum / notelenden;
}

void MEvalNOTE(MTrack *trk, MCell *cell)
{
  MCell *cp1, *cp2, *cp3;
  MBYTE note, vel1, vel2;
  int len1, len2, t;

  /* note number */
  cp1 = cell->val.cell;
  note = MEvalNOTENO( cp1 );
  /* note length */
  cp1 = cp1->next->next;
  len1 = MEvalNOTELEN( cp1 );
  /* velocity */
  cp2 = cp1->next;
  vel1 = MEvalBYTE( cp2->val.cell, curch->vel );
  if( vel1 > 0x7f ){
    MLINEERROR( linenum, MERROR_VELOUTOFRAN, NULL );
    return;
  }
  cp2 = cp2->next;
  cp3 = cp2->next;
  /* off */
  if( MCellIsCMD( cp3, MML_OFF ) ){
    if( cp1->val.cell || cp1->next->val.cell ||
        !MCellIsCMD( cp2, MML_INVALID ) )
      MLINEWARN( linenum, MERROR_ARG, NULL );
    vel2 = MEvalBYTE( cp3->val.cell->val.cell, 0 );
    if( vel2 > 0x7f ){
      MLINEERROR( linenum, MERROR_VELOUTOFRAN, NULL );
      return;
    }
    MEventListInsertMSG3( &trk->elist,
      curch->id, curch->time, 0x80, curch->ch, note, vel2 );
    return;
  }
  t = curch->time;
  MEventListInsertMSG3( &trk->elist,
    curch->id, t, 0x90, curch->ch, note, vel1 );
  /* hold */
  if( MCellIsCMD( cp2, MML_HOLD ) ){
    if( !MCellIsCMD( cp3, MML_INVALID ) )
      MLINEWARN( linenum, MERROR_ARG, NULL );
    cp2 = cp2->val.cell;
    if( cp2->val.cell == NULL ){
      MChannelTimeIncrement( curch, len1 );
      return;
    }
    len2 = MEvalNOTELEN( cp2 );
    if( len2 < 0 ){
      MLINEERROR( linenum, MERROR_NEGATIVELEN, NULL );
      return;
    }
    t += len1 + len2;
  } else
    t += len1 * curch->p.gt / curch->p.gtd;
  MEventListInsertMSG3( &trk->elist,
    curch->id, t, 0x80, curch->ch, note, 0 );
  MChannelTimeIncrement( curch, len1 );
}

void MEvalREST(MTrack *trk, MCell *cell)
{
  MChannelTimeIncrement( curch, MEvalNOTELEN( cell->val.cell ) );
}

MInt linearip(int s, int e, int l, int t){
  return (MInt)( s + ( (double)( e - s ) * t /l ) ); }
MInt quad1ip(int s, int e, int l, int t){
  return (MInt)( s - ( (double)( e - s ) * t*(t-2*l) /l /l ) ); }
MInt quad2ip(int s, int e, int l, int t){
  return (MInt)( s + ( (double)( e - s ) * t*t /l /l ) ); }
MInt cubicip(int s, int e, int l, int t){
  return (MInt)( s - ( (double)( e - s ) * t*t*(2*t-3*l) /l /l /l ) ); }

static MInt (* ip[])(int,int,int,int) = {
  linearip, quad1ip, quad2ip, cubicip
};

void MEvalEffect(MTrack *trk, MCell *cell, void (* f)(MTrack*,MInt,MInt,MBYTE), MBYTE arg, MInt maxval, MInt offset)
{
  register unsigned int i;
  MInt t, sval, eval, val, oldval, len, efflen, dlen, step;
  MBYTE type;

  /* start value */
  if( cell->val.cell == NULL ){
    MLINEERROR( linenum, MERROR_CTRLNO, NULL );
    return;
  }
  sval = MEvalVAL( cell->val.cell, 0 ) + offset;
  if( sval > maxval ){
    MLINEERROR( linenum, MERROR_VALOUTOFRAN, NULL );
    return;
  }
  f( trk, curch->time, sval, arg );
  oldval = sval;
  /* end value */
  cell = cell->next;
  if( cell->val.cell == NULL ) return; /* end */
  eval = MEvalVAL( cell->val.cell, 0 ) + offset;
  if( eval > maxval ){
    MLINEERROR( linenum, MERROR_VALOUTOFRAN, NULL );
    return;
  }
  /* effect length */
  cell = cell->next;
  efflen = MEvalNOTELEN( cell );
  /* proceed length */
  cell = cell->next;
  len = curch->p.len;
  curch->p.len = efflen;
  dlen = MEvalNOTELEN( cell );
  curch->p.len = len;
  /* effect gradation type */
  cell = cell->next;
  type = MEvalBYTE( cell->val.cell, 0 );
  if( type > 3 ){
    MLINEERROR( linenum, MERROR_CURVE, NULL );
    return;
  }
  /* gradation step */
  cell = cell->next;
  step = MEvalVAL( cell->val.cell, 1 );
  /* effect gradation */
  for( i=1; i<=efflen; i+=step ){
    t = curch->time + i;
    val = ip[type]( sval, eval, efflen, i );
    if( val > maxval ){
      MLINEERROR( linenum, MERROR_VALOUTOFRAN, NULL );
      return;
    }
    if( val == oldval ) continue;
    f( trk, t, val, arg );
    oldval = val;
  }
  MChannelTimeIncrement( curch, dlen );
}

static void polykeypress(MTrack *trk, MInt t, MInt val, MBYTE note);
void polykeypress(MTrack *trk, MInt t, MInt val, MBYTE note){
  MEventListInsertMSG3( &trk->elist,
    curch->id, t, 0xa0, curch->ch, note, (MBYTE)val );
}
void MEvalPOLYKEYPRESS(MTrack *trk, MCell *cell)
{
  MBYTE note;

  /* note no. */
  note = MEvalBYTE( cell->val.cell->val.cell, 0 );
  if( note > 0x7f){
    MLINEERROR( linenum, MERROR_NOTEOUTOFRAN, NULL );
    return;
  }
  /* effect */
  MEvalEffect( trk, cell->val.cell->next, polykeypress, note, 0x7f, 0 );
}

static void controlchange(MTrack *trk, MInt t, MInt val, MBYTE cn);
void controlchange(MTrack *trk, MInt t, MInt val, MBYTE cn){
  MEventListInsertMSG3( &trk->elist,
    curch->id, t, 0xb0, curch->ch, cn, (MBYTE)val );
}
void MEvalCONTROLCHANGE(MTrack *trk, MCell *cell)
{
  MBYTE cn;

  /* control no. */
  cn = MEvalBYTE( cell->val.cell->val.cell, 0 );
  if( cn > 0x7f){
    MLINEERROR( linenum, MERROR_VALOUTOFRAN, NULL );
    return;
  }
  /* effect */
  MEvalEffect( trk, cell->val.cell->next, controlchange, cn, 0x7f, 0 );
}

void MEvalPROGRAMCHANGE(MTrack *trk, MCell *cell)
{
  MBYTE pn, b1, b2;

  pn = MEvalBYTE( cell->val.cell->val.cell, 0 );
  if( pn == 0 || pn > 0x80){
    MLINEERROR( linenum, MERROR_VALOUTOFRAN, NULL );
    return;
  }
  if( cell->val.cell->next->val.cell ){
    b1 = MEvalBYTE( cell->val.cell->next->val.cell, 0 );
    b2 = MEvalBYTE( cell->val.cell->next->next->val.cell, 0 );
    MEventListInsertMSG3( &trk->elist,
      curch->id, curch->time, 0xb0, curch->ch, 0x00, b1 );
    MEventListInsertMSG3( &trk->elist,
      curch->id, curch->time, 0xb0, curch->ch, 0x20, b2 );
  }
  MEventListInsertMSG2( &trk->elist,
    curch->id, curch->time, 0xc0, curch->ch, (MBYTE)( pn-1 ) );
}

static void channelpress(MTrack *trk, MInt t, MInt val, MBYTE dummy);
void channelpress(MTrack *trk, MInt t, MInt val, MBYTE dummy){
  MEventListInsertMSG2( &trk->elist,
    curch->id, t, 0xd0, curch->ch, (MBYTE)val );
}
void MEvalCHANNELPRESS(MTrack *trk, MCell *cell)
{
  /* effect */
  MEvalEffect( trk, cell->val.cell, channelpress, 0, 0x7f, 0 );
}

static void pitchbend(MTrack *trk, MInt t, MInt val, MBYTE dummy);
void pitchbend(MTrack *trk, MInt t, MInt val, MBYTE dummy){
  static MBYTE lsb, msb;
  lsb = (MBYTE)(   val        & 0x7f );
  msb = (MBYTE)( ( val >> 7 ) & 0x7f );
  MEventListInsertMSG3( &trk->elist,
    curch->id, t, 0xe0, curch->ch, lsb, msb );
}
void MEvalPITCHBEND(MTrack *trk, MCell *cell)
{
  /* effect */
  MEvalEffect( trk, cell->val.cell, pitchbend, 0, 0x3fff, 0x2000 );
}

static void mastervolume(MTrack *trk, MInt t, MInt val, MBYTE dummy);
void mastervolume(MTrack *trk, MInt t, MInt val, MBYTE dummy){
  static MBYTE data[] = { 0x7f, 0x7f, 0x40, 0x01, 0x00, 0x00, 0xf7 };
  data[4] = data[5] = (MBYTE)val;
  MEventListInsertExclusiveMSG( &trk->elist, curch->id, t, 7, data );
}
void MEvalMASTERVOLUME(MTrack *trk, MCell *cell)
{
  /* effect */
  MEvalEffect( trk, cell->val.cell, mastervolume, 0, 0x7f, 0 );
}

static void tempo(MTrack *trk, MInt t, MInt val, MBYTE dummy);
void tempo(MTrack *trk, MInt t, MInt val, MBYTE dummy){
  static MBYTE tp[3];
  static unsigned long tempoval;
  tempoval = 60000000 / val;
  tp[0] = (MBYTE)( ( tempoval >> 16 ) & 0xff );
  tp[1] = (MBYTE)( ( tempoval >> 8  ) & 0xff );
  tp[2] = (MBYTE)(   tempoval         & 0xff );
  MEventListInsertMetaMSG( &trk->elist, curch->id, t, 0x51, 3, tp );
}
void MEvalTEMPO(MTrack *trk, MCell *cell)
{
  /* effect */
  MEvalEffect( trk, cell->val.cell, tempo, 0, 0xffffffff, 0 );
}

void MEvalTRANSPOSE(MTrack *trk, MCell *cell)
{
  curch->p.tr = (signed char)MEvalVAL( cell->val.cell->val.cell, curch->p.tr );
}

void MEvalBASENOTE(MTrack *trk, MCell *cell)
{
  curch->p.oct = (signed char)MEvalVAL( cell->val.cell->val.cell, curch->p.oct );
}

void MEvalDEFAULTLEN(MTrack *trk, MCell *cell)
{
  int len;
  
  len = MEvalNOTELEN( cell->val.cell );
  if( len < 0 ){
    MLINEERROR( linenum, MERROR_NEGATIVELEN, NULL );
    return;
  }
  curch->p.len = (MInt)len;
}

void MEvalDEFAULTVEL(MTrack *trk, MCell *cell)
{
  curch->vel = MEvalBYTE( cell->val.cell->val.cell, curch->vel );
  if( curch->vel > 0x7f )
    MLINEERROR( linenum, MERROR_VELOUTOFRAN, NULL );
}

void MEvalGATETIME(MTrack *trk, MCell *cell)
{
  curch->p.gt  = MEvalBYTE( cell->val.cell->val.cell, curch->p.gt );
  if( cell->val.cell->next ){
    curch->p.gtd = (MBYTE)MEvalINT( cell->val.cell->next->val.cell, curch->p.gtd );
    if( curch->p.gtd == 0 ){
      MLINEERROR( linenum, MERROR_ZEROGTD, NULL );
      MABORT();
    }
  }
}

void MEvalCHANGECHANNEL(MTrack *trk, MCell *cell)
{
  curch->ch = (MBYTE)( MEvalINT( cell->val.cell, 1 ) - 1 );
}

void MEvalDIVIDECHANNEL(MTrack *trk, MCell *cell)
{
  MInt headtime, endtime;
  MChannelParam p;
  MCell *env;

  headtime = endtime = curch->time;
  p = curch->p;
  for( env=cell->val.cell->val.cell; env; env=env->next ){
    if( env->val.cell == NULL ) continue;
    curch->time = headtime;
    MEvalCMDLIST( trk, env->val.cell );
    if( endtime < curch->time ) endtime = curch->time;
    curch->p = p;
  }
  MChannelTimeUpdate( curch, endtime );
}

void MEvalEXCLUSIVE(MTrack *trk, MCell *cell)
{
  register int i;
  MBYTE size;
  MBYTE *data;

  size = (MBYTE)( MCellListCount( cell->val.cell->val.cell ) + 1 );
  data = MALLOC( MBYTE, size );
  if( data == NULL )
    MALLOCERROR();

  for( i=0, cell=cell->val.cell->val.cell; cell; i++, cell=cell->next )
    data[i] = (MBYTE)MEvalINT( cell, 0 );
  data[i] = 0xf7; /* EOX */
  MEventListInsertExclusiveMSG( &trk->elist,
    curch->id, curch->time, size, data );
}

void MEvalGSEXCLUSIVE(MTrack *trk, MCell *cell)
{
  /* this function works only for GS modules */
  register int i;
  MBYTE size;
  MBYTE *data, checksum = 0;

  size = (MBYTE)( MCellListCount( cell->val.cell->val.cell ) + 2 );
  data = MALLOC( MBYTE, size );
  if( data == NULL )
    MALLOCERROR();

  for( i=0, cell=cell->val.cell->val.cell; cell; i++, cell=cell->next ){
    data[i] = (MBYTE)MEvalINT( cell, 0 );
    if( i >= 4 ) checksum += data[i];
  }
  data[i] = (MBYTE)( 0x80 - ( checksum & 0x7f ) );
  data[i+1] = 0xf7; /* EOX */
  MEventListInsertExclusiveMSG( &trk->elist,
    curch->id, curch->time, size, data );
}

void MEvalSEQUENCE(MTrack *trk, MCell *cell)
{
  MCell *ev;
  MInt headtime;
  MChannelParam p;
  int oldnum, oldden, totallen;

  /* memorize old values */
  oldnum = notelennum;
  oldden = notelenden;
  /* calclulate total length */
  totallen = 0;
  for( ev=cell->val.cell->val.cell; ev; ev=ev->next ){
    if( MCellIsCMD( ev, MML_NOTE ) )
      totallen += MEvalLEN( ev->val.cell->next->next->val.cell );
    else if( MCellIsCMD( ev, MML_REST ) )
      totallen += MEvalLEN( ev->val.cell->val.cell );
    else if( MCellIsCMD( ev, MML_CHORD_BEGIN ) )
      totallen += MEvalLEN( ev->val.cell->next->val.cell->val.cell );
    else if( MCellIsCMD( ev, MML_SEQUENCE ) )
      totallen += MEvalLEN( ev->val.cell->next->val.cell );
    else if( !MCellIsCMD( ev, MML_TRANSPOSE ) &&
             !MCellIsCMD( ev, MML_BASENOTE ) &&
             !MCellIsCMD( ev, MML_DEFAULTVEL ) &&
             !MCellIsCMD( ev, MML_GATETIME ) &&
             !MCellIsCMD( ev, MML_EXCLUSIVE ) &&
             !MCellIsCMD( ev, MML_GSEXCLUSIVE ) ){
      MLINEERROR( linenum, MERROR_SEQ, NULL );
      return;
    }
  }
  if( totallen < 0 ){
    MLINEERROR( linenum, MERROR_NEGATIVELEN, NULL );
    return;
  }
  /* real length */
  notelennum = MEvalNOTELEN( cell->val.cell->next );
  notelenden = totallen;
  /* update cmd length & evaluation */
  headtime = curch->time;
  p = curch->p;
  MEvalCMDLIST( trk, cell->val.cell->val.cell );
  curch->time = headtime;
  curch->p = p;
  MChannelTimeIncrement( curch, notelennum );
  notelennum = oldnum;
  notelenden = oldden;
}

static void MEvalCHORDArgCover(MCell *arg, MCell *cover);
void MEvalCHORDArgCover(MCell *arg, MCell *cover)
{
  /* note length */
  if( arg->val.cell == NULL )
    arg->val.cell = MCellCopy( cover->val.cell );
  arg = arg->next; cover = cover->next;
  /* velocity */
  if( arg->val.cell == NULL )
    arg->val.cell = MCellCopy( cover->val.cell );
  arg = arg->next; cover = cover->next;
  /* hold */
  if( arg->id - MML_CMD == MML_INVALID ){
    arg->id = cover->id;
    arg->val.cell = MCellCopy( cover->val.cell );
  }
  arg = arg->next; cover = cover->next;
  /* off */
  if( arg->id - MML_CMD == MML_INVALID ){
    arg->id = cover->id;
    arg->val.cell = MCellCopy( cover->val.cell );
  }
}

void MEvalCHORD(MTrack *trk, MCell *cell)
{
  MCell *cmd, *defarg;
  MInt headtime;
  MChannelParam p;

  headtime = curch->time;
  p = curch->p;
  defarg = cell->val.cell->next->val.cell;
  for( cmd=cell->val.cell->val.cell; cmd; cmd=cmd->next ){
    curch->time = headtime;
    if( MCellIsCMD( cmd, MML_NOTE ) )
      MEvalCHORDArgCover( cmd->val.cell->next->next, defarg );
    else if( MCellIsCMD( cmd, MML_CHORD_BEGIN ) )
      MEvalCHORDArgCover( cmd->val.cell->next->val.cell, defarg );
    MEvalCMD( trk, cmd );
  }
  curch->time = headtime;
  curch->p = p;

  if( !MCellIsCMD( defarg->next->next->next, MML_OFF ) )
    MChannelTimeIncrement( curch, MEvalNOTELEN( defarg ) );
}

void MEvalREPEAT(MTrack *trk, MCell *cell)
{
  MCell *rc, *ip;
  MChannelParam p;
  register MInt i, rn = 1, turn = 2;

  /* count repeat time */
  for( rc=cell->val.cell; rc; rc=rc->next ){
    if( rc->id == MML_ENV ) continue;
    if( MCellIsCMD( rc, MML_REPEAT_CASE ) ){
      ip = rc->val.cell->val.cell;
      if( ip == NULL )
        ip = rc->val.cell->val.cell = MCellCreateINT( rn++ );
      else
        rn = MEvalINT( ip, 0 ) + 1;
      for( ; ip; ip=ip->next ){
        i = MEvalINT( ip, 0 );
        if( i > turn ) turn = i;
      }
    } else if( MCellIsCMD( rc, MML_REPEAT_END ) ){
      i = MEvalVAL( rc->val.cell->val.cell, 2 );
      if( i < turn )
        MLINEWARN( linenum, MERROR_REPTIME, NULL );
      else
        if( i > turn ) turn = i;
      break;
    }
  }
  /* execute repeat */
  p = curch->p;
  for( i=1; i<=turn; i++ ){
    for( rc=cell->val.cell; rc; rc=rc->next )
      if( rc->id == MML_ENV )
        MEvalCMDLIST( trk, rc->val.cell );
      else if( MCellIsCMD( rc, MML_REPEAT_CASE ) ){
        for( ip=rc->val.cell->val.cell; ip; ip=ip->next )
          if( i == (MInt)MEvalINT( ip, 0 ) ) break;
        if( ip == NULL ) rc = rc->next;
      } else if( MCellIsCMD( rc, MML_REPEAT_END ) )
	break;
    curch->p = p;
  }
}

/* metarocess evaluation */

void MEvalMETATextEvent(MTrack *trk, MBYTE type, MString str)
{
  MEventListInsertMetaMSG( &trk->elist,
    curch ? curch->id : 0, curch ? curch->time : 0, type,
    (MBYTE)MStringLen(str), (MBYTE *)str );
}

void MEvalMETAChannelPrefix(MTrack *trk, MBYTE ch)
{
  static MBYTE array[1];

  array[0] = (MBYTE)( ch - 1 );
  curch = MChannelFind( ch );
  MEventListInsertMetaMSG( &trk->elist,
    curch ? curch->id : 0, curch ? curch->time : 0, 0x20, 1, array );
}

void MEvalMETAChannelEvent(MTrack *trk, MCell *cell, MBYTE type)
{
  MCell *cp;

  cp=cell->val.cell;
  if( MChannelFind( (MBYTE)cp->val.intval ) ){
    MEvalMETAChannelPrefix( trk, (MBYTE)cp->val.intval );
    cp = cp->next;
    MEvalMETATextEvent( trk, type, cp->val.expr );
  }
}

void MEvalTEXT(MTrack *trk, MCell *cell){
  MEvalMETATextEvent( trk, 0x01, cell->val.cell->val.expr ); }

void MEvalCOPYRIGHT(MTrack *trk, MCell *cell){
  MEvalMETATextEvent( trk, 0x02, cell->val.cell->val.expr ); }

void MEvalTITLE(MTrack *trk, MCell *cell){
  MEvalMETAChannelPrefix( trk, 1 );
  MEvalMETATextEvent( trk, 0x03, cell->val.cell->val.expr ); }

void MEvalTRACK(MTrack *trk, MCell *cell){
  MEvalMETAChannelEvent( trk, cell, 0x03 ); }

void MEvalINSTRUMENT(MTrack *trk, MCell *cell){
  MEvalMETAChannelEvent( trk, cell, 0x04 ); }

void MEvalLYRIC(MTrack *trk, MCell *cell){
  MEvalMETAChannelEvent( trk, cell, 0x05 ); }

void MEvalMARKER(MTrack *trk, MCell *cell){
  MEvalMETAChannelEvent( trk, cell, 0x06 ); }

void MEvalCUE(MTrack *trk, MCell *cell){
  MEvalMETAChannelEvent( trk, cell, 0x07 ); }

void MEvalFORMAT(MTrack *trk, MCell *cell)
{
  format = (MBYTE)cell->val.cell->val.intval;
  MFormatCheck();
}

void MEvalTIMEBASE(MTrack *trk, MCell *cell)
{
  timebase = cell->val.cell->val.intval;
  MTimebaseCheck();
}

void MEvalBEAT(MTrack *trk, MCell *cell)
{
  static MBYTE array[4];
  MBYTE flag = 0, bn;

  /* channel prefix */
  MEvalMETAChannelPrefix( trk, (MBYTE)cell->val.cell->val.intval );
  cell = cell->val.cell->next;
  /* beat evaluation */
  array[0] = (MBYTE)MEvalINT( cell, 0 );
  bn = (MBYTE)MEvalINT( cell->next->next, 4 );
  for( array[1]=0; bn>1; array[1]++, bn >>= 1 )
    if( bn % 2 ) flag = 1;
  if( flag )
    MLINEERROR( linenum, MERROR_BEAT, NULL );
  array[2] = 0x18;
  array[3] = 0x08;
  MEventListInsertMetaMSG( &trk->elist,
    curch ? curch->id : 0, curch ? curch->time : 0, 0x58, 4, array );
}

void MEvalCODE(MTrack *trk, MCell *cell)
{
  static MBYTE val[2];

  if(  MCodeFind( cell->val.cell->val.expr, val ) == 0 ){
    MLINEERROR( linenum, MERROR_CODE, cell->val.cell->val.expr );
    return;
  }
  MEventListInsertMetaMSG( &trk->elist,
    curch ? curch->id : 0, curch ? curch->time : 0, 0x59, 2, val );
}

void MEvalSYNC(MTrack *trk, MCell *cell)
{
  register int i;
  MInt t;
  MChannel *ch;

  ch = MChannelFind( (MBYTE)MEvalVAL( cell->val.cell->val.cell, 0 ) );
  t = ( ch == NULL ) ? maxtime : ch->time;

  for( i=0; i<channelnum; i++ )
    if( channel[i].time < t ) channel[i].time = t;
  mintime = MChannelMinTime();
}

void MEvalECHO(MTrack *trk, MCell *cell)
{
  if( cell->val.cell->val.expr )
    MECHO( cell->val.cell->val.expr );
}

void MEvalINCLUDE(MTrack *trk, MCell *cell)
{
  FILE *oldsrc;
  MString oldsrcname;

  oldsrc = src;
  oldsrcname = srcname;
  MCompileTrack( trk, cell->val.cell->val.expr );
  src = oldsrc;
  srcname = oldsrcname;
}

void MEvalDEFCMD(MTrack *trk, MCell *cell)
{
  MString key, expr;
  MCMD *cmd;

  key = cell->val.cell->val.expr;
  expr = cell->val.cell->next->val.expr;
  if( key == NULL ){
    MLINEERROR( linenum, MERROR_DEFCMD, NULL );
    return;
  }
  if( MStringLen( key ) != 1 ){
    MLINEERROR( linenum, MERROR_DEFKEY, key );
    return;
  }
  cmd = MCMDNameToCMD( expr );
  if( cmd == NULL ){
    MLINEERROR( linenum, MERROR_DEFUNKNOWN, expr );
    return;
  }
  MCMDSetKey( cmd, key[0] );
}

void MEvalDEFMACRO(MTrack *trk, MCell *cell)
{
  static char defbuf[MBUFSIZ];
  MCell *arg = NULL, *expr = NULL, *ap;
  MString cp, sp;
  MMacro *macro;

  if( cell->val.cell->val.expr == NULL ||
      cell->val.cell->next->val.expr == NULL ){
    MLINEERROR( linenum, MERROR_DEFMACRO, NULL );
    return;
  }
  /* get key & find (or create) macro */
  for( cp=defbuf, sp=cell->val.cell->val.expr;
       !MIsTerminator(*sp) && !MIsWS(*sp) && !MCMDCmp(MML_VAR,*sp);
       cp++, sp++ )
    *cp = *sp;
  *cp = '\0';
  macro = MMacroFind( defbuf );
  if( macro ){
    MLINEWARN( linenum, MERROR_OVERWRITE, defbuf );
    MCellDestroy( macro->arg );
    MCellDestroy( macro->expr );
    macro->arg = macro->expr = NULL;
  } else{
    macro = MMacroCreate( defbuf, NULL, NULL );
    MMacroListAdd( macro );
  }
  /* get argument list */
  while( MCMDCmp( MML_VAR, *sp ) ){
    for( sp++, cp=defbuf;
         !MIsTerminator(*sp) && !MIsWS(*sp) && !MCMDCmp(MML_VAR,*sp);
         cp++, sp++ )
      *cp = *sp;
    *cp = '\0';
    arg = MCellListAdd( arg, MCellCreateSTR( defbuf ) );
  }
  macro->arg = arg;
  /* get expr list */
  sp = cell->val.cell->next->val.expr;
  while( *sp )
    if( MCMDCmp( MML_VAR, *sp ) ){
      for( sp++, ap=arg; ap; ap=ap->next )
        if( MStringCmp( ap->val.expr, sp ) ) break;
      if( ap == NULL ){
        MLINEERROR( linenum, MERROR_MACROVAR, sp );
        return;
      }
      expr = MCellListAdd( expr, MCellCreatePTR( ap ) );
      sp += MStringLen( ap->val.expr );
    } else{
      for( cp=defbuf;
           !MIsTerminator(*sp) && !MIsWS(*sp) &&
           !MCMDCmp(MML_VAR,*sp);
           cp++, sp++ )
        *cp = *sp;
      *cp = '\0';
      while( MIsWS( *sp ) ) sp++;
      expr = MCellListAdd( expr, MCellCreateSTR( defbuf ) );
    }
  for( ap=arg; ap; ap=ap->next )
    MFREE_RESET( ap->val.expr );
  macro->expr = expr;
}

void MEvalMASK(MTrack *trk, MCell *cell)
{
  mask = 1;
  MRepeatStackPush( 0, MML_REPSTACK_MASK, 1 );
}

void MEvalUNMASK(MTrack *trk, MCell *cell)
{
  if( MRepeatStackIsEmpty() ||
      repeatstack.next->type != MML_REPSTACK_MASK ){
    MLINEWARN( linenum, MERROR_MASK, NULL );
    return;
  }
  MRepeatStackPop();
}

void MEvalMETAREPEAT(MTrack *trk, MCell *cell)
{
  MRepeatStackPush( MEvalVAL( cell->val.cell->val.cell, 2 ),
    MML_REPSTACK_REPEAT, MRepeatStackMask() );
}

void MEvalMETAREPEATCASE(MTrack *trk, MCell *cell)
{
  MCell *cp;

  if( MRepeatStackIsEmpty() ||
      repeatstack.next->type != MML_REPSTACK_REPEAT ){
    MLINEERROR( linenum, MERROR_METAREPEAT, NULL );
    return;
  }
  cp = cell->val.cell->val.cell;
  mask = 1;
  for( ; cp; cp=cp->next )
    if( cp->val.intval == repeatstack.next->count ){
      mask = MRepeatStackMask();
      break;
    }
}

void MEvalMETAREPEATEND(MTrack *trk, MCell *cell)
{
  if( MRepeatStackIsEmpty() ){
    MLINEWARN( linenum, MERROR_METAREPEAT, NULL );
    return;
  }
  if( repeatstack.next->type == MML_REPSTACK_CODA )
    return;
  if( repeatstack.next->type != MML_REPSTACK_REPEAT ){
    MLINEWARN( linenum, MERROR_METAREPEAT, NULL );
    return;
  }
  if( repeatstack.next->count >= repeatstack.next->maxcount )
    MRepeatStackPop();
  else{
    repeatstack.next->count++;
    fseek( src, repeatstack.next->seek_offset, SEEK_SET );
    linenum = repeatstack.next->lineno;
    mask = MRepeatStackMask();
  }
}

void MEvalFINE(MTrack *trk, MCell *cell)
{
  if( MRepeatStackIsEmpty() ||
      repeatstack.next->type != MML_REPSTACK_REPEAT ){
    MLINEWARN( linenum, MERROR_METAREPEAT, NULL );
    return;
  }
  if( repeatstack.next->count >= repeatstack.next->maxcount ){
    while( !MRepeatStackIsEmpty() )
      MRepeatStackPop();
    fseek( src, 0, SEEK_END );
  }
}

void MEvalTOCODA(MTrack *trk, MCell *cell)
{
  if( MRepeatStackIsEmpty() ||
      repeatstack.next->type != MML_REPSTACK_REPEAT ){
    MLINEWARN( linenum, MERROR_METAREPEAT, NULL );
    return;
  }
  if( repeatstack.next->count >= repeatstack.next->maxcount ){
    mask = 1;
    MRepeatStackPush( 0, MML_REPSTACK_CODA, 1 );
  }
}

void MEvalCODA(MTrack *trk, MCell *cell)
{
  if( MRepeatStackIsEmpty() ||
      repeatstack.next->type != MML_REPSTACK_CODA ){
    MLINEWARN( linenum, MERROR_CODA, NULL );
    return;
  }
  MRepeatStackPop();
  mask = MRepeatStackMask();
  MRepeatStackPop();
}

void MEvalPLAYLIST(MTrack *trk, MCell *cell)
{
  register int i;
  MCell *cp;

  cp = cell->val.cell->val.cell;
  MChannelCreate( MCellListCount( cp ) );
  for( i=0; i<channelnum; cp=cp->next, i++ ){
    if( cp->val.intval == 0 ){
      MLINEERROR( linenum, MERROR_TRACK, NULL );
      return;
    }
    if( format == 1 && cp->val.intval > (int)tracknum )
      tracknum += 16;
    channel[i].id = channel[i].ch = (MBYTE)( cp->val.intval - 1 );
  }
}

void MEvalUNPLAYLIST(MTrack *trk, MCell *cell)
{
  register int i;
  MCell *cp;

  if( channelnum == 0 ) return; /* ignored */
  for( cp=cell->val.cell->val.cell; cp; cp=cp->next ){
    if( cp->val.intval == 0 ){
      MLINEERROR( linenum, MERROR_TRACK, NULL );
      return;
    }
    for( i=0; i<channelnum; i++ )
      if( (int)channel[i].id == cp->val.intval - 1 )
        channel[i].id = 0xff;
  }
}
