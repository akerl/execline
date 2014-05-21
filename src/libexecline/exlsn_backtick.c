/* ISC license. */

#include <unistd.h>
#include "bytestr.h"
#include "sgetopt.h"
#include "strerr2.h"
#include "stralloc.h"
#include "genalloc.h"
#include "djbunix.h"
#include "execline.h"
#include "exlsn.h"

int exlsn_backtick (int argc, char const **argv, char const *const *envp, exlsn_t_ref info)
{
  eltransforminfo si = ELTRANSFORMINFO_ZERO ;
  subgetopt_t localopt = SUBGETOPT_ZERO ;
  elsubst blah ;
  int argc1 ;
  char const *x ;
  int insist = 0 ;
  blah.var = info->vars.len ;
  blah.value = info->values.len ;

  for (;;)
  {
    register int opt = subgetopt_r(argc, argv, "insCcd:", &localopt) ;
    if (opt < 0) break ;
    switch (opt)
    {
      case 'i' : insist = 1 ; break ;
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
  x = argv[0] ;
  argc-- ; argv++ ;
  argc1 = el_semicolon(argv) ;
  if (argc1 >= argc) return -3 ;
  if (!stralloc_cats(&info->vars, x)) return -1 ;
  if (!stralloc_0(&info->vars)) goto err ;
  {
    int p[2] ;
    int pid ;
    if (pipe(p) == -1) goto err ;
    switch ((pid = fork()))
    {
      case -1:
        fd_close(p[1]) ;
        fd_close(p[0]) ;
        goto err ;
      case 0:
        argv[argc1] = 0 ;
        fd_close(p[0]) ;
        if (fd_move(1, p[1]) == -1)
          strerr_diewu1sys(111, "move backtick's writer fd") ;
        pathexec0_run(argv, envp) ;
        strerr_diewu2sys(111, "spawn backtick's ", argv[0]) ;
    }
    fd_close(p[1]) ;
    if (!slurp(&info->values, p[0]))
    {
      fd_close(p[0]) ;
      goto err ;
    }
    fd_close(p[0]) ;
    if (wait_pid(&p[0], pid) == -1) goto err ;
    if (insist && wait_status(p[0]))
      strerr_dief1x(wait_status(p[0]), "child process exited non-zero") ;
  }
  {
    register int r = el_transform(&info->values, blah.value, &si) ;
    if (r < 0) goto err ;
    blah.n = r ;
  }
  if (!genalloc_append(elsubst, &info->data, &blah)) goto err ;
  return localopt.ind + argc1 + 2 ;

 err:
  info->vars.len = blah.var ;
  info->values.len = blah.value ;
  return -1 ;
}
