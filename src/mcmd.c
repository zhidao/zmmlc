#include <mcmd.h>

/* CMD name, ID, default key, argument list */
static MCMD cmdlist[] = {
  { "INVALID",       MML_INVALID,       '\0', NULL },
  { "METACOM",       MML_METACOM,       '#', NULL },
  { "VAR",           MML_VAR,           '@', NULL },
  { "PAR",           MML_PAR,           '/', NULL },
  { "CHANNEL",       MML_CHANNEL,       ':', NULL },
  { "COMMENT",       MML_COMMENT,       '%', NULL },
  { "NOTE",          MML_NOTE,          'n', NULL },
  { "REST",          MML_REST,          'r', NULL },
  { "POLYKEYPRESS",  MML_POLYKEYPRESS,  '_', NULL },
  { "CONTROLCHANGE", MML_CONTROLCHANGE, '?', NULL },
  { "PROGRAMCHANGE", MML_PROGRAMCHANGE, 'i', NULL },
  { "CHANNELPRESS",  MML_CHANNELPRESS,  '\'', NULL },
  { "PITCHBEND",     MML_PITCHBEND,     'P', NULL },
  { "MASTERVOLUME",  MML_MASTERVOLUME,  'V', NULL },
  { "TEMPO",         MML_TEMPO,         't', NULL },
  { "TRANSPOSE",     MML_TRANSPOSE,     'T', NULL },
  { "BASENOTE",      MML_BASENOTE,      'o', NULL },
  { "DEFAULTLEN",    MML_DEFAULTLEN,    'l', NULL },
  { "DEFAULTVEL",    MML_DEFAULTVEL,    'v', NULL },
  { "GATETIME",      MML_GATETIME,      'q', NULL },
  { "CHANGECHANNEL", MML_CHANGECHANNEL, 's', NULL },
  { "DIVIDECHANNEL", MML_DIVIDECHANNEL, 'D', NULL },
  { "EXCLUSIVE",     MML_EXCLUSIVE,     'E', NULL },
  { "GSEXCLUSIVE",   MML_GSEXCLUSIVE,   'G', NULL },
  { "MACROCALL",     MML_MACROCALL,     'M', NULL },
  { "SEQUENCE",      MML_SEQUENCE,      'S', NULL },
  { "CHORD_BEGIN",   MML_CHORD_BEGIN,   '{', NULL },
  { "CHORD_END",     MML_CHORD_END,     '}', NULL },
  { "REPEAT_BEGIN",  MML_REPEAT_BEGIN,  '[', NULL },
  { "REPEAT_CASE",   MML_REPEAT_CASE,   '/', NULL },
  { "REPEAT_END",    MML_REPEAT_END,    ']', NULL },
  { "ENV_BEGIN",     MML_ENV_BEGIN,     '{', NULL },
  { "ENV_END",       MML_ENV_END,       '}', NULL },
  { "OFF",           MML_OFF,           '!', NULL },
  { "HOLD",          MML_HOLD,          '&', NULL },
  { "INCREMENT",     MML_INCREMENT,     '+', NULL },
  { "DECREMENT",     MML_DECREMENT,     '-', NULL },
  { "TIE",           MML_TIE,           '^', NULL },
  { "SHARP",         MML_SHARP,         '+', NULL },
  { "FLAT",          MML_FLAT,          '-', NULL },
  { "HALF",          MML_HALF,          '.', NULL },
  { "PRECISION",     MML_PRECISION,     '`', NULL },
  { "HEXADECIMAL",   MML_HEXADECIMAL,   '$', NULL },
  { "QUOTATION",     MML_QUOTATION,     '\"', NULL },
  { "DELIMITER",     MML_DELIMITER,     ',', NULL },
  { "NL",            MML_NL,            '\\', NULL },
};

/* CMD reverse indices ... key -> ID */
static MBYTE cmdindex[MASCII_NUM];

/* number of CMD */
#define MML_CMD_NUM MML_NL

char MCMDKey(int id)
{
  return cmdlist[id].key;
}

int MCMDCmp(MBYTE id, char key)
{
  return MCMDKey(id) == key ? 1 : 0;
}

MString MCMDName(MBYTE id)
{
  return cmdlist[ id<=MML_NL ? id : MML_INVALID ].name;
}

MCMD *MCMDNameToCMD(MString name)
{
  register MBYTE i;

  if( name ) for( i=1; i<=MML_CMD_NUM; i++ )
    if( !strcmp( MCMDName( i ), name ) ) return &cmdlist[i];
  return NULL;
}

int MCMDIsIndependent(char key)
{
  return key>=MML_COMMENT && key<=MML_REPEAT_END ? 1 : 0;
}

void MCMDSetKey(MCMD *cmd, char key)
{
  cmd->key = key;
  if( MCMDIsIndependent( key ) )
    cmdindex[MASCII2ID(key)] = cmd->id;
}

MCMD *MCMDFind(MBYTE id)
{
  return id <= MML_NL ? &cmdlist[id] : NULL;
}

MCMD *MCMDFindKey(char key)
{
  return &cmdlist[cmdindex[MASCII2ID(key)]];
}

#ifdef DEBUG
void MCMDPrint(MCMD *cmd)
{
  if( cmd == NULL ){
    printf( "cmd:(null)\n" );
    return;
  }
  printf( "cmd:%s (Key='%c')\n", MCMDName(cmd->id), cmd->key );
  if( cmd->arg ){
    printf( "  (arg:)\n" );
    MCellPrint( cmd->arg, 2 );
  }
}

void MCMDListPrint(void)
{
  register int i;

  for( i=0; i<=MML_CMD_NUM; i++ )
    MCMDPrint( &cmdlist[i] );
}
#endif

/* built-in */

/* argument list */
static MCell notearg[8]; /* note & chord end */
static MCell pcarg[3];   /* program change family */
static MCell ccarg[7];   /* control change family */
static MCell dlarg;      /* default length family */
static MCell charg;      /* change channel */
static MCell exarg;      /* system exclusive family */
static MCell mcarg[2];   /* macro call */
static MCell dcarg;      /* divide channel */
static MCell sarg[2];    /* sequence */

void MCMDBuiltinInit(void)
{
  register MBYTE i, n;

  /* initialization of CMD arguments */

  /* note
     chord end */
  MCellInit( &notearg[0], MML_VAL );
  MCellInit( &notearg[1], MML_KEY );
  MCellInit( &notearg[2], MML_LEN );
  MCellInit( &notearg[3], MML_VAL );
  MCellInit( &notearg[4], MML_CMD );
  notearg[4].id = MML_CMD + MML_HOLD;
  MCellInit( &notearg[5], MML_CMD );
  notearg[5].id = MML_CMD + MML_OFF;
  for( i=0; i<5; i++ )
    notearg[i].next = &notearg[i+1];
  cmdlist[MML_NOTE].arg = notearg;
  cmdlist[MML_CHORD_END].arg = &notearg[2];

  /* program change
     transpose
     base note
     default velocity
     gate time
     repeat end
     note off */
  for( i=0; i<3; i++ ){
    MCellInit( &pcarg[i], MML_VAL );
    if( i < 2 ) pcarg[i].next = &pcarg[i+1];
  }
  cmdlist[MML_PROGRAMCHANGE].arg = &pcarg[0];
  cmdlist[MML_TRANSPOSE].arg = &pcarg[2];
  cmdlist[MML_BASENOTE].arg = &pcarg[2];
  cmdlist[MML_DEFAULTVEL].arg = &pcarg[2];
  cmdlist[MML_GATETIME].arg = &pcarg[1];
  cmdlist[MML_REPEAT_END].arg = &pcarg[2];
  cmdlist[MML_OFF].arg = &pcarg[2];

  /* polyphonic key pressure
     control change
     channel pressure
     pitchbend
     master volume
     tempo */
  MCellInit( &ccarg[0], MML_VAL );
  MCellInit( &ccarg[1], MML_VAL );
  MCellInit( &ccarg[2], MML_VAL );
  MCellInit( &ccarg[3], MML_LEN );
  MCellInit( &ccarg[4], MML_LEN );
  MCellInit( &ccarg[5], MML_VAL );
  MCellInit( &ccarg[6], MML_VAL );
  for( i=0; i<6; i++ )
    ccarg[i].next = &ccarg[i+1];
  cmdlist[MML_POLYKEYPRESS].arg = &ccarg[0];
  cmdlist[MML_CONTROLCHANGE].arg = &ccarg[0];
  cmdlist[MML_CHANNELPRESS].arg = &ccarg[1];
  cmdlist[MML_PITCHBEND].arg = &ccarg[1];
  cmdlist[MML_MASTERVOLUME].arg = &ccarg[1];
  cmdlist[MML_TEMPO].arg = &ccarg[1];

  /* default length
     rest
     hold */
  MCellInit( &dlarg, MML_LEN );
  cmdlist[MML_REST].arg = &dlarg;
  cmdlist[MML_DEFAULTLEN].arg = &dlarg;
  cmdlist[MML_HOLD].arg = &dlarg;

  /* change channel */
  MCellInit( &charg, MML_INT );
  cmdlist[MML_CHANGECHANNEL].arg = &charg;

  /* system exclusive
     repeat case */
  MCellInit( &exarg, MML_INTLIST );
  cmdlist[MML_EXCLUSIVE].arg = &exarg;
  cmdlist[MML_GSEXCLUSIVE].arg = &exarg;
  cmdlist[MML_REPEAT_CASE].arg = &exarg;

  /* macro call */
  MCellInit( &mcarg[0], MML_STR );
  MCellInit( &mcarg[1], MML_STRLIST );
  mcarg[0].next = &mcarg[1];
  cmdlist[MML_MACROCALL].arg = mcarg;

  /* divide channel */
  MCellInit( &dcarg, MML_ENVLIST );
  cmdlist[MML_DIVIDECHANNEL].arg = &dcarg;

  /* sequence */
  MCellInit( &sarg[0], MML_ENV );
  MCellInit( &sarg[1], MML_LEN );
  sarg[0].next = &sarg[1];
  cmdlist[MML_SEQUENCE].arg = sarg;

  /* initialization of reverse indices */
  n = MASCII_NUM;
  for( i=0; i<n; i++ )
    cmdindex[i] = MML_INVALID;
  for( i=MML_COMMENT; i<=MML_REPEAT_END; i++ )
    cmdindex[MASCII2ID(cmdlist[i].key)] = i;
}
