#include <mmeta.h>
#include <mcmd.h>

/* meta command name, ID, argument list */
static MMeta metalist[] = {
  { "invalid",    MML_INVALID,         NULL },
  { "sync",       MML_META_SYNC,       NULL },
  { "echo",       MML_META_ECHO,       NULL },
  { "define",     MML_META_DEFMACRO,   NULL },
  { "def",        MML_META_DEFCMD,     NULL },
  { "mask",       MML_META_MASK,       NULL },
  { "unmask",     MML_META_UNMASK,     NULL },
  { "repeat",     MML_META_REPEAT,     NULL },
  { "repcase",    MML_META_REPEATCASE, NULL },
  { "repend",     MML_META_REPEATEND,  NULL },
  { "fine",       MML_META_FINE,       NULL },
  { "tocoda",     MML_META_TOCODA,     NULL },
  { "coda",       MML_META_CODA,       NULL },
  { "include",    MML_META_INCLUDE,    NULL },
  { "marker",     MML_META_MARKER,     NULL },
  { "cue",        MML_META_CUE,        NULL },
  { "text",       MML_META_TEXT,       NULL },
  { "lyric",      MML_META_LYRIC,      NULL },
  { "track",      MML_META_TRACK,      NULL },
  { "instrument", MML_META_INSTRUMENT, NULL },
  { "beat",       MML_META_BEAT,       NULL },
  { "code",       MML_META_CODE,       NULL },
  { "format",     MML_META_FORMAT,     NULL },
  { "timebase",   MML_META_TIMEBASE,   NULL },
  { "play",       MML_META_PLAYLIST,   NULL },
  { "unplay",     MML_META_UNPLAYLIST, NULL },
  { "copyright",  MML_META_COPYRIGHT,  NULL },
  { "title",      MML_META_TITLE,      NULL },
};

/* number of meta command */
#define MML_META_NUM MML_META_TITLE

/* argument list */
static MCell snarg;    /* sync & repeat case */
static MCell cearg[2]; /* channel event family */
static MCell btarg[4]; /* beat family */
static MCell plarg;    /* play list & unplay list */
static MCell dfarg[2]; /* CMD definition & macro definition */

void MMetaInit(void)
{
  /* [channel event]
     track name
     instrument name
     lyric
     marker
     cue point
     [text event]
     title
     text
     copyright
     code
     include
     echo */
  MCellInit( &cearg[0], MML_INT );
  cearg[0].next = &cearg[1];
  MCellInit( &cearg[1], MML_STR );
  metalist[MML_META_MARKER].arg     = cearg;
  metalist[MML_META_CUE].arg        = cearg;
  metalist[MML_META_TRACK].arg      = cearg;
  metalist[MML_META_INSTRUMENT].arg = cearg;
  metalist[MML_META_LYRIC].arg      = cearg;
  metalist[MML_META_ECHO].arg      = &cearg[1];
  metalist[MML_META_INCLUDE].arg   = &cearg[1];
  metalist[MML_META_TEXT].arg      = &cearg[1];
  metalist[MML_META_CODE].arg      = &cearg[1];
  metalist[MML_META_COPYRIGHT].arg = &cearg[1];
  metalist[MML_META_TITLE].arg     = &cearg[1];
  /* beat
     format
     timebase */
  MCellInit( &btarg[0], MML_INT );
  btarg[0].next = &btarg[1];
  MCellInit( &btarg[1], MML_INT );
  btarg[1].next = &btarg[2];
  MCellInit( &btarg[2], MML_CMD );
  btarg[2].id = MML_CMD + MML_PAR;
  btarg[2].next = &btarg[3];
  MCellInit( &btarg[3], MML_INT );
  metalist[MML_META_BEAT].arg = btarg;
  metalist[MML_META_FORMAT].arg = &btarg[3];
  metalist[MML_META_TIMEBASE].arg = &btarg[3];

  /* sync
     repeat */
  MCellInit( &snarg, MML_VAL );
  metalist[MML_META_SYNC].arg = &snarg;
  metalist[MML_META_REPEAT].arg = &snarg;

  /* play list
     unplay list */
  MCellInit( &plarg, MML_INTLIST );
  metalist[MML_META_PLAYLIST].arg = &plarg;
  metalist[MML_META_UNPLAYLIST].arg = &plarg;
  metalist[MML_META_REPEATCASE].arg = &plarg;
  /* CMD definition
     macro definition */
  MCellInit( &dfarg[0], MML_STR );
  dfarg[0].next = &dfarg[1];
  MCellInit( &dfarg[1], MML_STR );
  metalist[MML_META_DEFMACRO].arg = dfarg;
  metalist[MML_META_DEFCMD].arg = dfarg;
}

MString MMetaName(MBYTE id)
{
  return metalist[ id <= MML_META_NUM ? id : MML_INVALID ].name;
}

MMeta *MMetaFind(MString key)
{
  register int i;

  for( i=MML_META_SYNC; i<=MML_META_TITLE; i++ )
    if( !strcmp( key, metalist[i].name ) ) return &metalist[i];
  return NULL;
}

#ifdef DEBUG
void MMetaPrint(MMeta *meta)
{
  if( meta == NULL ){
    printf( "(null) " );
    return;
  }
  printf( "Metacommand (Key=\"%s\")\n", MMetaName( meta->id ) );
  if( meta->arg ){
    printf( "  (arg:)\n" );
    MCellPrint( meta->arg, 2 );
  }
}

void MMetaListPrint(void)
{
  register int i;

  for( i=0; i<=MML_META_NUM; i++ )
    MMetaPrint( &metalist[i] );
}
#endif

/* code name */

typedef struct{
  MString name;
  char n;
  MBYTE m;
} MCode;

static MCode code[] = {
  /* major code */
  { "G-Major",-6, 0 },
  { "D-Major",-5, 0 },
  { "A-Major",-4, 0 },
  { "E-Major",-3, 0 },
  { "B-Major",-2, 0 },
  { "FMajor", -1, 0 },
  { "CMajor",  0, 0 },
  { "GMajor",  1, 0 },
  { "DMajor",  2, 0 },
  { "AMajor",  3, 0 },
  { "EMajor",  4, 0 },
  { "BMajor",  5, 0 },
  { "F+Major", 6, 0 },
  { "C+Major", 7, 0 },
  { "G+Major", 8, 0 },
  { "D+Major", 9, 0 },
  { "A+Major",10, 0 },
  { "E+Major",11, 0 },
  { "B+Major",12, 0 },
  /* minor code */
  { "G-Minor",-9, 1 },
  { "D-Minor",-8, 1 },
  { "A-Minor",-7, 1 },
  { "E-Minor",-6, 1 },
  { "B-Minor",-5, 1 },
  { "FMinor", -4, 1 },
  { "CMinor", -3, 1 },
  { "GMinor", -2, 1 },
  { "DMinor", -1, 1 },
  { "AMinor",  0, 1 },
  { "EMinor",  1, 1 },
  { "BMinor",  2, 1 },
  { "F+Minor", 3, 1 },
  { "C+Minor", 4, 1 },
  { "G+Minor", 5, 1 },
  { "D+Minor", 6, 1 },
  { "A+Minor", 7, 1 },
  { NULL, 0, 0 }
};

int MCodeFind(MString name, MBYTE val[])
{
  MCode *cp;

  for( cp=code; cp->name; cp++ )
    if( !strcmp( cp->name, name ) ){
      val[0] = cp->n;
      val[1] = cp->m;
      return 1;
    }
  return 0;
}
