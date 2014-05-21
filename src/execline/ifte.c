/* ISC license. */

#include <unistd.h>
#include "sgetopt.h"
#include "fmtscan.h"
#include "strerr2.h"
#include "djbunix.h"
#include "execline.h"

#define USAGE "ifte [ -X ] [ -n ] { command-then... } { command-else... } command-if..."

int main (int argc, char const **argv, char const *const *envp)
{
  int argc1, argc2, wstat ;
  int not = 0, flagnormalcrash = 0 ;
  int pid ;
  PROG = "ifte" ;
  {
    subgetopt_t l = SUBGETOPT_ZERO ;
    for (;;)
    {
      register int opt = subgetopt_r(argc, argv, "Xn", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'X' : flagnormalcrash = 1 ; break ;
        case 'n' : not = 1 ; break ;
        default : strerr_dieusage(100, USAGE) ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  argc1 = el_semicolon(argv) ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated then block") ;
  if (argc1 + 1 == argc) strerr_dief1x(100, "else block required") ;
  argc2 = el_semicolon(argv + argc1 + 1) ;
  if (argc1 + argc2 + 1 >= argc) strerr_dief1x(100, "unterminated else block") ;
  if (argc1 + argc2 + 2 >= argc) strerr_dief1x(100, "empty command-if") ;

  pid = fork() ;
  switch (pid)
  {
    case -1: strerr_diefu1sys(111, "fork") ;
    case 0:
    {
      PROG = "ifte (child)" ;
      pathexec_run(argv[argc1 + argc2 + 2], argv + argc1 + argc2 + 2, envp) ;
      strerr_diefu2sys(111, "spawn ", argv[argc1 + argc2 + 2]) ;
    }
  }
  if (wait_pid(&wstat, pid) == -1)
    strerr_diefu2sys(111, "wait for ", argv[argc1 + argc2 + 2]) ;
  if (!flagnormalcrash && wait_crashed(wstat))
  {
    char fmt[UINT_FMT] ;
    fmt[uint_fmt(fmt, wait_stopsig(wstat))] = 0 ;
    strerr_dief2x(1, "child crashed with signal ", fmt) ;
  }
  if (not != !wait_status(wstat)) argv[argc1] = 0 ;
  else
  {
    argv += argc1 + 1 ;
    argv[argc2] = 0 ;
  }
  pathexec0_run(argv, envp) ;
  strerr_dieexec(111, *argv) ;
}
