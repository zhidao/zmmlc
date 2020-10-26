#include <mcell.h>
#ifdef DEBUG
#include <mcmd.h>
#include <mmeta.h>
#endif

/* MCell */
void MCellInit(MCell *cell, MBYTE id)
{
  cell->id = id;
  cell->val.expr = NULL;
  cell->next = NULL;
}

MCell *MCellCreate(MBYTE id)
{
  MCell *cell;

  cell = MALLOC( MCell, 1 );
  if( cell )
    MCellInit( cell, id );
  else
    MALLOCERROR();
  return cell;
}

void MCellDestroy(MCell *cell)
{
  if( cell == NULL ) return;
  switch( cell->id ){
  case MML_INT: case MML_INT2:
    cell->val.intval = 0;
  case MML_STR:
    MFREE( cell->val.expr ); break;
  case MML_PTR:
    cell->val.cell = NULL; break;
  case MML_INVALID:
  case MML_VAL: case MML_LEN: case MML_KEY: case MML_ENV:
  case MML_INTLIST: case MML_STRLIST: case MML_ENVLIST:
  default:
    MCellDestroy( cell->val.cell ); break;
  }
  MCellDestroy( cell->next );
  MFREE( cell );
}

MCell *MCellCreateSTR(MString str)
{
  MCell *cell;

  cell = MCellCreate( MML_STR );
  cell->val.expr = MStringAlloc( str );
  return cell;
}

MCell *MCellCreateINT(MInt val)
{
  MCell *cell;

  cell = MCellCreate( MML_INT );
  cell->val.intval = val;
  return cell;
}

/* MML_INT2 is only for MML_PRECISION */
/* ordinary int value is within 0 - 127 */
/* precision value is within 0 - 8191 */
MCell *MCellCreateINT2(MInt val)
{
  MCell *cell;

  cell = MCellCreate( MML_INT2 );
  cell->val.intval = val;
  return cell;
}

MCell *MCellCreatePTR(MCell *arg)
{
  MCell *cell;

  cell = MCellCreate( MML_PTR );
  cell->val.cell = arg;
  return cell;
}

MCell *MCellCopy(MCell *org)
{
  MCell *cell;

  if( org == NULL ) return NULL;
  cell = MCellCreate( org->id );
  switch( org->id ){
  case MML_INT: case MML_INT2:
    cell->val.intval = org->val.intval;
    break;
  case MML_STR:
    cell->val.expr = MStringAlloc( org->val.expr );
    break;
  case MML_PTR:
    cell->val.cell = org->val.cell;
  case MML_VAL: case MML_LEN: case MML_KEY: case MML_ENV:
  case MML_INTLIST: case MML_STRLIST: case MML_ENVLIST:
  default:
    cell->val.cell = MCellCopy( org->val.cell );
    break;
  }
  cell->next  = MCellCopy( org->next );
  return cell;
}

MBYTE MCellListCount(MCell *list)
{
  MCell *cp;
  MBYTE count;

  for( count=0, cp=list; cp; cp=cp->next, count++ );
  return count;
}

MCell *MCellListAdd(MCell *list, MCell *cell)
{
  MCell *cp;

  if( list == NULL ) return cell;
  for( cp=list; cp->next; cp=cp->next );
  cp->next = cell;
  return list;
}

#ifdef DEBUG
static MString cellname[] = {
  "INVALID",
  "INT", "INT2", "VAL", "LEN", "KEY", "STR",
  "ENV",
  "INTLIST", "STRLIST", "ENVLIST",
  "*PTR", "CMD", "META",
};

void MCellPrint(MCell *cell, int indent)
{
  MIndent( indent );
  if( cell == NULL ){
    printf( "(null)\n" );
    return;
  }
  switch( cell->id ){
  case MML_VAL:
  case MML_LEN:
  case MML_KEY:
  case MML_ENV:
  case MML_INTLIST:
  case MML_STRLIST:
  case MML_ENVLIST:
    printf( "[%s]\n", cellname[cell->id] );
    MCellPrint( cell->val.cell, indent+2 );
    break;
  case MML_STR:
    printf( "[%s] ", cellname[cell->id] );
    if( cell->val.expr ) printf( "\"%s\"", cell->val.expr );
    printf( "\n" );
    break;
  case MML_INT:
  case MML_INT2:
    printf( "[%s] %d\n", cellname[cell->id], cell->val.intval );
    break;
  case MML_PTR:
    printf( "[%s]\n", cellname[cell->id] );
    MCellPrint( cell->val.cell, indent+1 );
    break;
  default:
    if( cell->id >= MML_CMD && cell->id < MML_META ){
      /* cell is cmd */
      printf( "[CMD] " );
      printf( "%s", MCMDName( cell->id - MML_CMD ) );
      if( cell->val.cell ){
        printf( "\n%*s(child:)\n", indent+2, "" );
        MCellPrint( cell->val.cell, indent+2 );
      } else
        printf( "\n" );
    } else if( cell->id >= MML_META ){
      /* cell is meta */
      printf( "[META] " );
      printf( "%s", MMetaName( cell->id - MML_META ) );
      if( cell->val.cell ){
        printf( "\n%*s(child:)\n", indent+2, "" );
        MCellPrint( cell->val.cell, indent+2 );
      } else
        printf( "\n" );
    } else
      printf( "(invalid cell)\n" );
    break;
  }
  if( cell->next )
    MCellPrint( cell->next, indent );
}
#endif
