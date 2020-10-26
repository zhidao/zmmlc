#include <ctype.h>
#include <mstring.h>

#ifdef DEBUG
void MIndent(int indent)
{
  printf( "%*s", indent, "" );
}
#endif

MString MStringCat(MString dest, MString src)
{
  if( src ) strcat( dest, src );
  return dest;
}

int MStringLen(MString str)
{
  return str ? strlen( str ) : 0;
}

int MStringCmp(MString s1, MString s2)
{
  return !strncmp( s1, s2, MStringLen( s1 ) );
}

MString MStringAlloc(MString str)
{
  MString strbuf;

  strbuf = MALLOC( char, MStringLen(str) + 1 );
  if( strbuf == NULL )
    MALLOCERROR();

  strcpy( strbuf, str );
  return strbuf;
}

MString MStringI(MInt i)
{
#define MINTBUFSIZ 3
  static char intbuf[MINTBUFSIZ];

  sprintf( intbuf, "%d", i );
  return intbuf;
}

MString MStringHex(MBYTE h)
{
#define MHEXBUFSIZ 3
  static char hexbuf[MHEXBUFSIZ];

  sprintf( hexbuf, "%1X%1X", h >> 4, h & 0xf );
  return hexbuf;
}

#define MSUFFIXSIZ 6
int MFileNameHasSuffix(MString name, MString suffix)
{
  MString cp, sp;
  static char suf1[MSUFFIXSIZ], suf2[MSUFFIXSIZ];

  for( cp=suf1; *suffix; ) *cp++ = (char)tolower( *suffix++ );
  *cp = '\0';
  for( cp=name+strlen(name); cp>name; cp-- )
    if( *cp=='.' ){
      for( sp=suf2; *cp; cp++, sp++ ) *sp = (char)tolower( *cp );
      *sp = '\0';
      return !strncmp( suf1, suf2, strlen(suf1) ) ? 1 : 0;
    }
  return 0;
}

void MFileNameAddSuffix(MString name, MString suffix)
{
  if( !MFileNameHasSuffix( name, suffix ) )
    strcat( name, suffix );
}
