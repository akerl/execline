/* ISC license. */

#ifndef SUBGETOPTNOSHORT
#define SUBGETOPTNOSHORT
#endif

#include "sgetopt.h"
#include "fmtscan.h"
#include "exlsn.h"

int exlsn_exlp (int argc, char const **argv, char const *const *envp, exlsn_t_ref info)
{
  subgetopt_t localopt = SUBGETOPT_ZERO ;
  unsigned int nmin = 0 ;
  int n ;
  for (;;)
  {
    register int opt = subgetopt_r(argc, argv, "P:", &localopt) ;
    if (opt < 0) break ;
    switch (opt)
    {
      case 'P' : if (uint0_scan(localopt.arg, &nmin)) break ;
      default : return -3 ;
    }
  }
  argc -= localopt.ind ; argv += localopt.ind ;
  if (!argc) return -3 ;
  n = exlp(nmin, envp, info) ;
  if (n < 0) return n ;
  return localopt.ind ;
}
