#include <ctype.h>
#include <mparse.h>

FILE *src = NULL;
MString srcname;

MInt linenum = 0;

/* parsing state ID  */
#define MML_BASE      0
#define MML_IN_CHORD  1
#define MML_IN_REPEAT 2
#define MML_IN_ENV    3

/* state stack */

typedef struct __mstatestack{
  MBYTE state;
  struct __mstatestack *next;
} MStateStack;
static MStateStack statestack = { MML_BASE, NULL };

void MStatePush(MBYTE state)
{
  MStateStack *stack;

  stack = MALLOC( MStateStack, 1 );
  if( stack == NULL )
    MALLOCERROR();

  stack->state = state;
  stack->next = statestack.next;
  statestack.next = stack;
}

void MStatePop(void)
{
  MStateStack *stack;

  stack = statestack.next;
  if( stack ){
    statestack.next = stack->next;
    free( stack );
  } else
    statestack.next = NULL;
}

#define MState() ( statestack.next ? statestack.next->state : MML_BASE )

/* buffer & tokenization */

typedef struct __mbufstackcell{
  char buf[MBUFSIZ];
  char *bufcur;
  struct __mbufstackcell *prev;
} MBufStackCell;

typedef struct{
  MBufStackCell base, *head;
} MBufStack;

void MBufStackCellInit(MBufStackCell *cell)
{
  cell->buf[0] = '\0';
  cell->bufcur = cell->buf;
  cell->prev = NULL;
}

void MBufStackInit(MBufStack *stack)
{
  MBufStackCellInit( &stack->base );
  stack->head = &stack->base;
}

void MBufStackPush(MBufStack *stack)
{
  MBufStackCell *cell;

  if( ( cell = MALLOC( MBufStackCell, 1 ) ) == NULL )
    MALLOCERROR();
  MBufStackCellInit( cell );
  cell->prev = stack->head;
  stack->head = cell;
}

int MBufStackPop(MBufStack *stack)
{
  MBufStackCell *cell;

  if( stack->head == &stack->base ) /* stack is empty */
    return 0;
  cell = stack->head;
  stack->head = cell->prev;
  MFREE( cell );
  return 1;
}

void MBufStackStrCpy(MBufStack *stack, char *str)
{
  strcpy( stack->head->buf, str );
}

MBufStack bstack;
char token[MBUFSIZ];

#define MBufCur()       bstack.head->bufcur
#define MBufCurInc()    bstack.head->bufcur++

#define MBufPutBack() ( bstack.head->bufcur -= MStringLen( token ) )

void MOpenSrc(MString name)
{
  static char filename[MFILENAMELEN];

  strcpy( filename, name );
  MFileNameAddSuffix( filename, ".mml" );
  if( verbose )
    MECHOOPENMML( filename );
  src = fopen( filename, "r" );
  if( src == NULL )
    MOPENERROR( filename );

  srcname = MStringAlloc( filename );
}

void MCloseSrc(void)
{
  fclose( src );
  MFREE( srcname );
}

MString MGetBuf(void)
{
  if( feof( src ) ) return NULL;
  MBufStackInit( &bstack );
  linenum++;
  return fgets( bstack.base.buf, MBUFSIZ, src ) ? bstack.base.buf : NULL;
}

int MIsTerminator(char c)
{
  return ( c >= 0x0 && c <= 0x1f && c != '\t' ) || c == 0x7f;
}

int MIsWS(char c)
{
  return c == ' '  || c == '\t';
}

MString MSkipWS(void)
{
  while( MIsWS( *MBufCur() ) ) MBufCurInc();
  return MBufCur();
}

MString MGetToken(void)
{
  MString cp;
  MMacro *macro;
  MCell *arg;

  while( 1 ){
    MSkipWS();
    cp = token;
    if( MCMDCmp( MML_NL, *MBufCur() ) ){
      if( MGetBuf() == NULL ){
        *cp = '\0';
        break;
      }
      continue;
    }
    if( isdigit( *MBufCur() ) )
      while( isdigit( *MBufCur() ) ) *cp++ = *MBufCurInc();
    else if( MIsTerminator( *MBufCur() ) ){
      if( MBufStackPop( &bstack ) ) continue;
    } else
      *cp++ = *MBufCurInc();
    *cp = '\0';
    macro = MMacroFind( token );
    /* macro expansion */
    if( macro == NULL ) break;
    arg = MGetMacroArg( macro );
    MGetMacro( macro, arg );
    MCellDestroy( arg );
  }
  return token;
}

MString MGetString(void)
{
  MString cp;

  MSkipWS();
  cp = token;
  if( MCMDCmp( MML_NL, *MBufCur() ) ){
    if( MGetBuf() == NULL ){
      *cp = '\0';
      return token;
    }
    MSkipWS();
  }
  if( MCMDCmp( MML_ENV_BEGIN, *MBufCur() ) ) MBufCurInc();
  if( MCMDCmp( MML_QUOTATION, *MBufCur() ) ){
    MBufCurInc();
    while( !MIsTerminator( *MBufCur() ) ){
      if( MCMDCmp( MML_QUOTATION, *MBufCur() ) ){
        MBufCurInc();
        break;
      }
      if( MCMDCmp( MML_NL, *MBufCur() ) ){
        MBufCurInc();
        if( MIsTerminator( *MBufCur() ) ) break;
      }
      *cp++ = *MBufCurInc();
    }
  } else
    while( !MIsTerminator( *MBufCur() ) &&
           !MIsWS( *MBufCur() ) &&
           !MCMDCmp( MML_DELIMITER, *MBufCur() ) &&
           !MCMDCmp( MML_ENV_END,   *MBufCur() ) ){
      *cp++ = *MBufCurInc();
    }
  *cp = '\0';
  return token;
}

/* hexadecimal number */

MString MGetHex(void)
{
  MString cp;

  MSkipWS();
  cp = token;
  while( isdigit( *MBufCur() ) ||
      ( *MBufCur() >= 'a' && *MBufCur() <= 'f' ) ||
      ( *MBufCur() >= 'A' && *MBufCur() <= 'F' ) ) *cp++ = *MBufCurInc();
  *cp = '\0';
  return token;
}

static MInt MHexToSInt(char hex)
{
  if( isdigit( hex ) ) return hex - '0';
  if( hex >= 'a' && hex <= 'f' )
    return hex - 'a' + 10;
  if( hex >= 'A' && hex <= 'F' )
    return hex - 'A' + 10;
  return 0;
}

MInt MHexToInt(MString hex)
{
  MInt value = 0;

  if( hex == NULL ) return 0;
  while( *hex ){
    value <<= 4;
    value |= MHexToSInt( *hex );
    hex++;
  }
  return value;
}

static MCell* (* MGetCell[])(void) = {
  NULL,
  MGetINT, MGetINT, MGetVAL, MGetLEN, MGetKEY, MGetSTR, 
  MGetENV, MGetINTLIST, MGetSTRLIST, MGetENVLIST,
  NULL, NULL, NULL
};

MCell *MGetINT(void)
{
  MCell *cell = NULL;
  MCell *(* cellcreatef)(MInt) = MCellCreateINT;

  MGetToken();
  if( MCMDCmp( MML_PRECISION, token[0] ) ){
    cellcreatef = MCellCreateINT2;
    MGetToken();
  }
  if( MCMDCmp( MML_HEXADECIMAL, token[0] ) )
    cell = cellcreatef( MHexToInt( MGetHex() ) );
  else if( isdigit( token[0] ) )
    cell = cellcreatef( atoi(token) );
  else
    MBufPutBack();
  return cell;
}

MCell *MGetVAL(void)
{
  MCell *cell = NULL;

  MGetToken();
  if( MCMDCmp( MML_INCREMENT, token[0] ) )
    cell = MCellCreateCMD( MML_INCREMENT );
  else if( MCMDCmp( MML_DECREMENT, token[0] ) )
    cell = MCellCreateCMD( MML_DECREMENT );
  else
    MBufPutBack();
  cell = MCellListAdd( cell, MGetINT() );
  return cell;
}

MCell *MGetLEN(void)
{
  MCell *head = NULL, *cell;
  MBYTE prev = MML_INVALID;

  while( 1 ){
    MGetToken();
    if( MCMDCmp( MML_TIE, token[0] ) ){
      head = MCellListAdd( head, MCellCreateCMD( MML_TIE ) );
      prev = MML_TIE;
    } else
    if( MCMDCmp( MML_HALF, token[0] ) ){
      head = MCellListAdd( head, MCellCreateCMD( MML_HALF ) );
      prev = MML_HALF;
    } else {
      MBufPutBack();
      if( prev != MML_TIE && prev != MML_INVALID )
        break;
      cell = MCellCreate( MML_VAL );
      cell->val.cell = MGetVAL();
      if( cell->val.cell == NULL ){
        MCellDestroy( cell );
        break;
      }
      prev = MML_HALF;
      head = MCellListAdd( head, cell );
    }
  }
  return head;
}

MCell *MGetKEY(void)
{
  MCell *cell = NULL;

  MGetToken();
  if( MCMDCmp( MML_SHARP, token[0] ) )
    cell = MCellCreateCMD( MML_SHARP );
  else if( MCMDCmp( MML_FLAT, token[0] ) )
    cell = MCellCreateCMD( MML_FLAT );
  else
    MBufPutBack();
  return cell ? MCellListAdd( cell, MGetKEY() ) : NULL;
}

MCell *MGetSTR(void)
{
  MGetString();
  return token[0] != '\0' ? MCellCreateSTR( token ) : NULL;
}

MCell *MGetCMD(MCMD *cmd)
{
  if( cmd->key != MGetToken()[0] ){
    MBufPutBack();
    return NULL;
  }
  return MGetCMDWithARG( cmd );
}

MCell *MGetCHORD(MCMD *cmd)
{
  MCell *head, *arg;

  MStatePush( MML_IN_CHORD );
  head = MCellCreateCMD( cmd->id );
  head->val.cell = MCellCreate( MML_ENV );
  while( 1 ){
    arg = MParseCMD();
    if( arg == NULL ){
      MLINEERROR( linenum, MERROR_MISSINGCHO, NULL );
      MCellDestroy( head );
      return NULL;
    }
    if( MCellIsCMD( arg, MML_CHORD_END ) ){
      MCellListAdd( head->val.cell, arg );
      break;
    }
    head->val.cell->val.cell =
      MCellListAdd( head->val.cell->val.cell, arg );
  }
  MStatePop();
  return head;
}

MCell *MGetREPEAT(MCMD *cmd)
{
  MCell *head, *ptr, *arg;

  MStatePush( MML_IN_REPEAT );
  head = MCellCreateCMD( cmd->id );
  head->val.cell = ptr = MCellCreate( MML_ENV );
  while( 1 ){
    arg = MParseCMD();
    if( arg == NULL ){
      MLINEERROR( linenum, MERROR_MISSINGREP, NULL );
      MCellDestroy( head );
      return NULL;
    }
    if( MCellIsCMD( arg, MML_REPEAT_CASE ) ){
      MCellListAdd( head->val.cell, arg );
      ptr = MCellCreate( MML_ENV );
      MCellListAdd( head->val.cell, ptr );
    } else
    if( MCellIsCMD( arg, MML_REPEAT_END ) ){
      MCellListAdd( head->val.cell, arg );
      break;
    } else
      ptr->val.cell = MCellListAdd( ptr->val.cell, arg );
  }
  MStatePop();
  return head;
}

MCell *MGetCMDWithARG(MCMD *cmd)
{
  MCell *cell;

  switch( cmd->id ){
  case MML_COMMENT:
    *MBufCur() = '\0';
    return NULL;
  case MML_CHORD_BEGIN:
    return MGetCHORD( cmd );
  case MML_REPEAT_BEGIN:
    return MGetREPEAT( cmd );
  case MML_ENV_BEGIN:
    MStatePush( MML_IN_ENV );
    break;
  case MML_CHORD_END:
    if( MState() != MML_IN_CHORD )
      MLINEERROR( linenum, MERROR_MISMATCHCHO, NULL );
    break;
  case MML_REPEAT_CASE:
    if( MState() != MML_IN_REPEAT ){
      token[0] = MCMDKey( MML_REPEAT_CASE );
      MLINEERROR( linenum, MERROR_REPCASE, token );
    }
    break;
  case MML_REPEAT_END:
    if( MState() != MML_IN_REPEAT )
      MLINEERROR( linenum, MERROR_MISMATCHREP, NULL );
    break;
  default: ;
  }
  cell = MCellCreateCMD( cmd->id );
  cell->val.cell = MGetARG( cmd->arg );
  return cell;
}

MCell *MGetMETA(void)
{
  MString cp;
  MMeta *meta;
  MCell *cell;

  MSkipWS();
  cp = token;
  while( !MIsTerminator( *MBufCur() ) && !MIsWS( *MBufCur() ) )
    *cp++ = *MBufCurInc();
  *cp = '\0';
  meta = MMetaFind( token );
  if( meta == NULL ){
    MLINEERROR( linenum, MERROR_UNKNOWNMETA, token );
    return NULL;
  }
  cell = MCellCreateMETA( meta->id );
  cell->val.cell = MGetARG( meta->arg );
  return cell;
}

MCell *MGetENV(void)
{
  if( !MCMDCmp( MML_ENV_BEGIN, MGetToken()[0] ) ){
    MBufPutBack();
    return NULL;
  }
  MStatePush( MML_IN_ENV );
  return MParseChannel();
}

MCell *MGetINTLIST(void)
{
  MCell *head, *cell;

  if( MCMDCmp( MML_ENV_BEGIN, *MBufCur() ) ) MBufCurInc();
  head = cell = NULL;
  do{
    if( MCMDCmp( MML_QUOTATION, *MBufCur() ) )
      cell = MStringToINTLIST( MGetString() );
    else
      cell = MGetINT();
    if( MCMDCmp( MML_DELIMITER, *MBufCur() ) ){
      MBufCurInc();
      if( cell == NULL )
        cell = MCellCreate( MML_INVALID );
    }
    head = MCellListAdd( head, cell );
  } while( cell != NULL );
  if( MCMDCmp( MML_ENV_END, *MBufCur() ) ) MBufCurInc();
  return head;
}

MCell *MGetSTRLIST(void)
{
  MCell *head, *cell;

  head = cell = NULL;
  do{
    cell = MGetSTR();
    if( MCMDCmp( MML_DELIMITER, *MBufCur() ) ){
      MBufCurInc();
      if( cell == NULL )
        cell = MCellCreate( MML_INVALID );
    }
    head = MCellListAdd( head, cell );
  } while( cell != NULL );
  if( MCMDCmp( MML_ENV_END, *MBufCur() ) ) MBufCurInc();
  return head;
}

MCell *MGetENVLIST(void)
{
  MCell *head, *cell;

  head = cell = NULL;
  while( 1 ){
    cell = MCellCreate( MML_ENV );
    cell->val.cell = MGetENV();
    if( cell->val.cell == NULL ) break;
    head = MCellListAdd( head, cell );
  }
  return head;
}

MCell *MGetARG(MCell *list)
{
  MCell *arg = NULL, *cell;

  for( ; list; list=list->next ){
    switch( list->id ){
    case MML_INT:
    case MML_STR:
      cell = MGetCell[list->id]();
      if( cell == NULL )
        cell = MCellCreate( list->id );
      break;
    default:
      if( list->id >= MML_CMD && list->id < MML_META ){
        /* cmd case */
        cell = MGetCMD( MCMDFind( (MBYTE)( list->id - MML_CMD ) ) );
        if( cell == NULL )
          cell = MCellCreateCMD( MML_INVALID );
        break;
      } else{
        cell = MCellCreate( list->id );
        if( MGetCell[list->id] )
          cell->val.cell = MGetCell[list->id]();
      }
    }
    arg = MCellListAdd( arg, cell );
    MSkipWS();
    if( list->id != MML_KEY && MCMDCmp( MML_DELIMITER, *MBufCur() ) )
      MBufCurInc();
  }
  return arg;
}

MCell *MStringToINTLIST(MString str)
{
  MCell *cell = NULL;

  for( ; *str; str++ )
    cell = MCellListAdd( cell, MCellCreateINT( (MInt)*str ) );
  return cell;
}

void MGetMacro(MMacro *macro, MCell *arg)
{
  MBufStackPush( &bstack );
  MMacroExpand( macro, arg, bstack.head->buf );
  MMacroClearArg( macro );
}

MCell *MGetMacroArg(MMacro *macro)
{
  MCell *arg, *cell = NULL;

  for( arg=macro->arg; arg; arg=arg->next ){
    cell = MCellListAdd( cell, MGetSTR() );
    if( MCMDCmp( MML_DELIMITER, *MBufCur() ) ) MBufCurInc();
  }
  return cell;
}

/* parsing */

MBYTE mask = 0;

MCell *MParseLine(void)
{
  MCell *cell, *chno, *chcell;

  if( MCMDCmp( MML_METACOM, *MBufCur() ) ){
    MBufCurInc();
    return MGetMETA();
  }
  if( !mask && isdigit( *MBufCur() ) ){
    chno = MGetINTLIST();
    if( MCMDCmp( MML_CHANNEL, *MBufCur() ) ){
      MBufCurInc();
      cell = MCellCreateCMD( MML_CHANNEL );
      cell->val.cell = chno;
      chcell = MParseChannel();
      if( MState() == MML_IN_ENV )
        MLINEWARN( linenum, MERROR_MISSINGENV, NULL );
      return MCellListAdd( cell, chcell );
    }
    MCellDestroy( chno );
  }
  return NULL;
}

MCell *MParseCMD(void)
{
  MCell *cell;
  MCMD *cmd;
  MMacro *macro;

 RETRY:
  MGetToken();
  if( MIsTerminator( token[0] ) ) return NULL;
  if( MCMDCmp( MML_ENV_END, token[0] ) && MState()==MML_IN_ENV ){
    MStatePop();
    return NULL;
  }
  cmd = MCMDFindKey( token[0] );
  if( cmd == NULL ){
    MLINEERROR( linenum, MERROR_UNKNOWNCMD, token );
    return NULL;
  }
  cell = MGetCMDWithARG( cmd );
  if( cell && MCellIsCMD( cell, MML_MACROCALL ) ){
    /* macro expansion */
    macro = MMacroFind( cell->val.cell->val.expr );
    if( macro == NULL )
      MLINEERROR( linenum, MERROR_UNKNOWNMACRO, cell->val.cell->val.expr );
    else
      MGetMacro( macro, cell->val.cell->next->val.cell );
    MCellDestroy( cell );
    goto RETRY;
  }
  return cell;
}

MCell *MParseChannel(void)
{
  MCell *head = NULL, *cell;

  while( 1 ){
    cell = MParseCMD();
    if( cell == NULL ) break;
    head = MCellListAdd( head, cell );
  }
  return head;
}
