/*
 * ZMMLC - Z's MML Compiler
 *
 * (C)Copyright Zhidao, since 2001. 9.15.
 * all rights are reserved.
 * Last updated 2004. 5.16.
 */

#include <mcompile.h>

static MString outnamep = NULL, srcnamep = NULL;

/* initialization and termination */

#define MML_OPTIONIDENT '-'

void zmmlcArgs(int argc, char *argv[])
{
  register int i;

  for( i=1; i<argc; i++ ){
    if( argv[i][0] == MML_OPTIONIDENT )
      switch( argv[i][1] ){
      case 'h': /* help */
        MUSAGE();
      case 'v': /* verbose */
        verbose = 1;
        break;
      case 'f': /* SMF format */
        format = (MBYTE)atoi( &argv[i][2] );
        MFormatCheck();
        break;
      case 'T': /* time base */
        timebase = atoi( &argv[i][2] );
        MTimebaseCheck();
        break;
      case 'o': /* output SMF name */
        if( ++i == argc ){
          MERROR( MERROR_OUTPUTFILE );
          exit( 1 );
        }
        outnamep = argv[i];
        break;
      default:  /* unknown option */
        MERRORWORD( MERROR_UNKNOWNOPTION, argv[i] );
        exit( 1 );
      }
    else
      srcnamep = argv[i];
  }
}

void zmmlcInit(void)
{
  MCMDBuiltinInit();
  MMetaInit();
}

void zmmlcDestroy(void)
{
  /* dummy */
}

void zmmlcCompile(void)
{
  if( srcnamep == NULL ){
    MUSAGE();
    exit( 1 );
  }
  MCompile( srcnamep, outnamep ? outnamep : (char *)"out" );
}

/* main */

int main(int argc, char *argv[])
{
  zmmlcArgs( argc, argv );
  if( verbose ) MZMMLC();
  zmmlcInit();
  zmmlcCompile();
  zmmlcDestroy();
  return 0;
}
