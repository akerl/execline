/* ISC license. */

#ifndef EXECLINE_H
#define EXECLINE_H

#include "gccattributes.h"
#include "stralloc.h"

#define EXECLINE_BLOCK_QUOTE_CHAR ' '
#define EXECLINE_BLOCK_END_CHAR '\0'

#ifdef EXECLINE_OLD_VARNAMES
# define LASTPID_NAME "LASTPID"
# define LASTPID_NAMELEN 7
# define LASTEXITCODE_NAME "LASTEXITCODE"
# define LASTEXITCODE_NAMELEN 12
#else
# define LASTPID_NAME "!"
# define LASTPID_NAMELEN 1
# define LASTEXITCODE_NAME "?"
# define LASTEXITCODE_NAMELEN 1
#endif


/* Basics */

extern int el_vardupl (char const *, char const *, unsigned int) gccattr_pure ;
extern unsigned int el_getstrict (void) gccattr_const ;
extern void el_obsolescent (void) ;


/* Environment shifting */

extern int el_pushenv (stralloc *, char const *const *, unsigned int, char const *const *, unsigned int) ;
extern int el_popenv  (stralloc *, char const *const *, unsigned int, char const *const *, unsigned int) ;


/* Sequence */

extern void el_execsequence (char const *const *, char const *const *, char const *const *) gccattr_noreturn ;


/* Block unquoting */

extern int el_semicolon (char const **) ;


/* Value transformation */

typedef struct eltransforminfo eltransforminfo, *eltransforminfo_ref ;
struct eltransforminfo
{
  char const *delim ;
  unsigned int crunch : 1 ;
  unsigned int chomp : 1 ;
  unsigned int split : 1 ;
} ;

#define ELTRANSFORMINFO_ZERO { " \n\r\t", 0, 0, 0 }

extern int el_transform (stralloc *, unsigned int, eltransforminfo const *) ;


/* Substitution */

typedef struct elsubst_s elsubst, *elsubst_ref ;
struct elsubst_s
{
  unsigned int var ;
  unsigned int value ;
  unsigned int n ;
} ;

extern int el_substitute (stralloc *, char const *, unsigned int, char const *, char const *, elsubst const *, unsigned int) ;

#endif
