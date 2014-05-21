/* ISC license. */

#include <unistd.h>
#include "sgetopt.h"
#include "fmtscan.h"
#include "strerr2.h"
#include "djbunix.h"
#include "execline.h"

#define USAGE "ifthenelse [ -X ] { command-if... } { command-then... } { command-else... } [ remainder... ]"

int main (int argc, char const **argv, char const *const *envp)
{
  int argc1, argc2, argc3, wstat ;
  int magicscope = 0, flagnormalcrash = 0 ;
  int pid ;
  PROG = "ifthenelse" ;
  {
    subgetopt_t l = SUBGETOPT_ZERO ;
    for (;;)
    {
      register int opt = subgetopt_r(argc, argv, "Xs", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'X' : flagnormalcrash = 1 ; break ;
        case 's' : magicscope = 1 ; break ;
        default : strerr_dieusage(100, USAGE) ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  argc1 = el_semicolon(argv) ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated if block") ;
  if (argc1 + 1 == argc) strerr_dief1x(100, "then block required") ;
  argc2 = el_semicolon(argv + argc1 + 1) ;
  if (argc1 + argc2 + 1 >= argc) strerr_dief1x(100, "unterminated then block") ;
  argc3 = el_semicolon(argv + argc1 + argc2 + 2) ;
  if (argc1 + argc2 + argc3 + 2 >= argc)
    strerr_dief1x(100, "unterminated else block") ;

  pid = fork() ;
  switch (pid)
  {
    case -1: strerr_diefu1sys(111, "fork") ;
    case 0:
    {
      PROG = "ifthenelse (if child)" ;
      argv[argc1] = 0 ;
      pathexec0_run(argv, envp) ;
      strerr_diefu2sys(111, "spawn ", argv[0]) ;
    }
  }
  if (wait_pid(&wstat, pid) == -1)
    strerr_diefu2sys(111, "wait for ", argv[0]) ;
  argv += argc1 + 1 ;
  {
    char const *const *remainder = argv + argc2 + argc3 + 2 ;
    if (!flagnormalcrash && wait_crashed(wstat))
    {
      char fmt[UINT_FMT] ;
      fmt[uint_fmt(fmt, wait_stopsig(wstat))] = 0 ;
      strerr_dief2x(1, "child crashed with signal ", fmt) ;
    }
    if (wait_status(wstat))
    {
      argv += argc2 + 1 ;
      argc2 = argc3 ;
    }
    if (magicscope)  /* undocumented voodoo - dangerous and powerful */
    {
      register unsigned int i = 0 ;
      for (; remainder[i] ; i++) argv[argc2+i] = remainder[i] ;
      argv[argc2+i] = 0 ;
      pathexec0_run(argv, envp) ;
      strerr_dieexec(111, argv[0]) ;
    }
    else
    {
      argv[argc2] = 0 ;
      el_execsequence(argv, remainder, envp) ;
    }
  }
}
