/* ISC license. */

#include "strerr2.h"
#include "execline.h"
#include "exlsn.h"

void exlsn_main (int argc, char const **argv, char const *const *envp, exlsnfunc_t_ref func, char const *USAGE)
{
  exlsn_t info = EXLSN_ZERO ;
  int r = (*func)(argc, argv, envp, &info) ;
  if (r < 0) switch (r)
  {
    case -3 : strerr_dieusage(100, USAGE) ;
    case -2 : strerr_dief1x(111, "bad substitution key") ;
    case -1 : strerr_diefu1sys(111, "complete exlsn function") ;
    default : strerr_diefu2x(111, "complete exlsn function", ": unknown error") ;
  }
  el_substandrun(argc-r, argv+r, envp, &info) ;
}
