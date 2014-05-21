/* ISC license. */

#include <unistd.h>
#include "fmtscan.h"
#include "sgetopt.h"
#include "strerr2.h"
#include "env.h"
#include "djbunix.h"
#include "execline.h"

#define USAGE "background [ -d ] { command... }"

int main (int argc, char const **argv, char const *const *envp)
{
  int pid, argc1 ;
  int df = 0 ;
  PROG = "background" ;
  {
    subgetopt_t l = SUBGETOPT_ZERO ;
    for (;;)
    {
      register int opt = subgetopt_r(argc, argv, "d", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'd' : df = 1 ; break ;
        default : strerr_dieusage(100, USAGE) ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  argc1 = el_semicolon(argv) ;
  if (!argc1) strerr_dief1x(100, "empty block") ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;
  if (argc1 + 1 == argc) df = 0 ;

  pid = df ? doublefork() : fork() ;
  switch (pid)
  {
    case -1: strerr_diefu2sys(111, df ? "double" : "", "fork") ;
    case 0:
      PROG = "background (child)" ;
      argv[argc1] = 0 ;
      pathexec0_run(argv, envp) ;
      strerr_dieexec(111, argv[0]) ;
  }
  if (argc1 + 1 == argc) return 0 ;
  {
    char fmt[LASTPID_NAMELEN + 1 + UINT_FMT] = LASTPID_NAME "=" ;
    register unsigned int i = LASTPID_NAMELEN + 1 ;
    i += uint_fmt(fmt+i, pid) ; fmt[i++] = 0 ;
    pathexec_r(argv + argc1 + 1, envp, env_len(envp), fmt, i) ;
  }
  strerr_dieexec(111, argv[argc1+1]) ;
}
