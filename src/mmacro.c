#include <mmacro.h>

void MMacroInit(MMacro *macro)
{
  macro->key = NULL;
  macro->arg = NULL;
  macro->expr = NULL;
  macro->next = NULL;
}

void MMacroDestroy(MMacro *macro)
{
  if( macro == NULL ) return;
  MFREE( macro->key );
  MCellDestroy( macro->expr );
  MCellDestroy( macro->arg );
  MMacroDestroy( macro->next );
  MFREE( macro );
}

static MMacro macrolist[MASCII_NUM];
static MBYTE macrolist_num;

void MMacroListInit(void)
{
  register int i;

  macrolist_num = MASCII_NUM;
  for( i=0; i<macrolist_num; i++ )
    MMacroInit( &macrolist[i] );
}

void MMacroListAdd(MMacro *macro)
{
  MMacro *p;

  for( p=&macrolist[MASCII2ID(macro->key[0])]; p->next; p=p->next );
  p->next = macro;
}

MMacro *MMacroCreateSimple(MString key, MString expr)
{
  MMacro *macro;

  macro = MALLOC( MMacro, 1 );
  if( macro == NULL )
    MALLOCERROR();

  MMacroInit( macro );
  macro->key = MStringAlloc( key );
  macro->expr = MCellCreateSTR( expr );
  return macro;
}

MMacro *MMacroCreate(MString key, MCell *arg, MCell *expr)
{
  MMacro *macro;

  macro = MALLOC( MMacro, 1 );
  if( macro == NULL )
    MALLOCERROR();

  MMacroInit( macro );
  macro->key = MStringAlloc( key );
  macro->arg = arg;
  macro->expr = expr;
  return macro;
}

MMacro *MMacroFind(MString key)
{
  MMacro *macro;

  for( macro=macrolist[MASCII2ID(key[0])].next;
       macro; macro=macro->next )
    if( !strcmp( macro->key, key ) )
      return macro;
  return NULL;
}

MString MMacroExpand(MMacro *macro, MCell *arg, MString dest)
{
  MCell *ap, *expr;

  for( ap=macro->arg; ap && arg; ap=ap->next, arg=arg->next )
    ap->val.expr = arg->val.expr;
  MStringClear( dest );
  for( expr=macro->expr; expr; expr=expr->next ){
    switch( expr->id ){
    case MML_STR:
      MStringCat( dest, expr->val.expr );
      break;
    case MML_PTR:
      MStringCat( dest, expr->val.cell->val.expr );
      break;
    default:
      MINNERERROR();
    }
  }
  return dest;
}

void MMacroClearArg(MMacro *macro)
{
  MCell *arg;

  for( arg=macro->arg; arg; arg=arg->next )
    switch( arg->id ){
    case MML_INT: case MML_INT2:
      arg->val.intval = 0; break;
    default:
      arg->val.expr = NULL;
    }
}

#ifdef DEBUG
void MMacroPrint(MMacro *macro)
{
  if( macro == NULL ){
    printf( "(null) " );
    return;
  }
  printf( "Macro (Key=\"%s\")\n", macro->key );
  if( macro->arg ){
    printf( "  (arg:)\n" );
    MCellPrint( macro->arg, 2 );
  }
  if( macro->expr ){
    printf( "  (expr:)\n" );
    MCellPrint( macro->expr, 2 );
  }
}

void MMacroListPrint(MMacro *macro)
{
  MMacroPrint( macro );
  if( macro && macro->next )
    MMacroListPrint( macro->next );
}
#endif

/* built-in */

void MMacroBuiltinInit(void)
{
  register int i;
  MCell *nrpnarg[3], *nrpnexpr[6];

  MMacroListInit();

  /* built-in macro */
  /* note A */
  MMacroListAdd( MMacroCreateSimple( "a", "n+9," ) );
  /* note B */
  MMacroListAdd( MMacroCreateSimple( "b", "n+11," ) );
  /* note C */
  MMacroListAdd( MMacroCreateSimple( "c", "n+0," ) );
  /* note D */
  MMacroListAdd( MMacroCreateSimple( "d", "n+2," ) );
  /* note E */
  MMacroListAdd( MMacroCreateSimple( "e", "n+4," ) );
  /* note F */
  MMacroListAdd( MMacroCreateSimple( "f", "n+5," ) );
  /* note G */
  MMacroListAdd( MMacroCreateSimple( "g", "n+7," ) );
  /* modulation */
  MMacroListAdd( MMacroCreateSimple( "m", "?$01," ) );
  /* panpot */
  MMacroListAdd( MMacroCreateSimple( "p", "?$0A," ) );
  /* expression */
  MMacroListAdd( MMacroCreateSimple( "x", "?$0B," ) );
  /* pitch bend sensitivity */
  MMacroListAdd( MMacroCreateSimple( "A", "?$65,0 ?$64,0 ?$06," ) );
  /* chorus send level */
  MMacroListAdd( MMacroCreateSimple( "C", "?$5D," ) );
  /* delay send level */
  MMacroListAdd( MMacroCreateSimple( "L", "?$5E," ) );
  /* NRPN */
  for( i=0; i<3; i++ ){
    nrpnarg[i] = MCellCreate( MML_STR );
    if( i > 0 ) nrpnarg[i-1]->next = nrpnarg[i];
  }
  nrpnexpr[0] = MCellCreateSTR( "?$63," );
  nrpnexpr[1] = MCellCreatePTR( nrpnarg[0] );
  nrpnexpr[2] = MCellCreateSTR( "?$62," );
  nrpnexpr[3] = MCellCreatePTR( nrpnarg[1] );
  nrpnexpr[4] = MCellCreateSTR( "?$06," );
  nrpnexpr[5] = MCellCreatePTR( nrpnarg[2] );
  for( i=1; i<6; i++ )
    nrpnexpr[i-1]->next = nrpnexpr[i];
  MMacroListAdd( MMacroCreate( "N", nrpnarg[0], nrpnexpr[0] ) );
  /* reverb send level */
  MMacroListAdd( MMacroCreateSimple( "R", "?$5B," ) );
  /* modulation sensitivity */
  MMacroListAdd( MMacroCreateSimple( "W", "?$65,0 ?$64,5 ?$06," ) );
  /* channel volume */
  MMacroListAdd( MMacroCreateSimple( "X", "?$07," ) );
  /* all note off */
  MMacroListAdd( MMacroCreateSimple( ";", "?$7B,0 " ) );
  /* all sound off */
  MMacroListAdd( MMacroCreateSimple( "*", "?$78,0 " ) );
  /* velocity increment */
  MMacroListAdd( MMacroCreateSimple( ")", "v+8 " ) );
  /* velocity decrement */
  MMacroListAdd( MMacroCreateSimple( "(", "v-8 " ) );
  /* upper octava */
  MMacroListAdd( MMacroCreateSimple( ">", "o+1 " ) );
  /* lower octava */
  MMacroListAdd( MMacroCreateSimple( "<", "o-1 " ) );
}

void MMacroBuiltinDestroy(void)
{
  register int i;

  for( i=0; i<macrolist_num; i++ ){
    MMacroDestroy( macrolist[i].next );
    macrolist[i].next = NULL;
  }
}
