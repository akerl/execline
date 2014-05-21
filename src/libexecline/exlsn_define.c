/* ISC license. */

#ifndef SUBGETOPTNOSHORT
#define SUBGETOPTNOSHORT
#endif

#include "sgetopt.h"
#include "stralloc.h"
#include "genalloc.h"
#include "execline.h"
#include "exlsn.h"

int exlsn_define (int argc, char const **argv, char const *const *envp, exlsn_t_ref info)
{
  eltransforminfo si = ELTRANSFORMINFO_ZERO ;
  subgetopt_t localopt = SUBGETOPT_ZERO ;
  elsubst blah ;
  blah.var = info->vars.len ;
  blah.value = info->values.len ;
  for (;;)
  {
    register int opt = subgetopt_r(argc, argv, "nsCcd:", &localopt) ;
    if (opt < 0) break ;
    switch (opt)
    {
      case 'n' : si.chomp = 1 ; break ;
      case 's' : si.split = 1 ; break ;
      case 'C' : si.crunch = 1 ; break ;
      case 'c' : si.crunch = 0 ; break ;
      case 'd' : si.delim = localopt.arg ; break ;
      default : return -3 ;
    }
  }
  argc -= localopt.ind ; argv += localopt.ind ;
 
  if (argc < 2) return -3 ;
  if (!*argv[0] || el_vardupl(argv[0], info->vars.s, info->vars.len)) return -2 ;
  if (!stralloc_cats(&info->vars, argv[0])) return -1 ;
  if (!stralloc_0(&info->vars)) goto err ;
  if (!stralloc_cats(&info->values, argv[1])) goto err ;
  {
    register int r = el_transform(&info->values, blah.value, &si) ;
    if (r < 0) goto err ;
    blah.n = r ;
  }
  if (!genalloc_append(elsubst, &info->data, &blah)) goto err ;
  (void)envp ;
  return localopt.ind + 2 ;

 err:
  info->vars.len = blah.var ;
  info->values.len = blah.value ;
  return -1 ;
}
