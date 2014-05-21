/* ISC license. */

#include <unistd.h>
#include <sys/wait.h>
#include "sgetopt.h"
#include "fmtscan.h"
#include "strerr2.h"
#include "djbunix.h"
#include "execline.h"

#define USAGE "if [ -n ] [ -X ] [ -t | -x exitcode ] { command... }"

int main (int argc, char const **argv, char const *const *envp)
{
  int argc1, wstat ;
  int pid ;
  unsigned int not = 0 ;
  unsigned short e = 1 ;
  int flagnormalcrash = 0 ;
  PROG = "if" ;
  {
    subgetopt_t l = SUBGETOPT_ZERO ;
    for (;;)
    {
      register int opt = subgetopt_r(argc, argv, "nXtx:", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'n' : not = 1 ; break ;
        case 'X' : flagnormalcrash = 1 ; break ;
        case 't' : e = 0 ; break ;
        case 'x' : if (ushort_scan(l.arg, &e)) break ;
        default : strerr_dieusage(100, USAGE) ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  argc1 = el_semicolon(argv) ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;
  pid = fork() ;
  switch (pid)
  {
    case -1: strerr_diefu1sys(111, "fork") ;
    case 0:
      argv[argc1] = 0 ;
      pathexec0_run(argv, envp) ;
      strerr_diefu2sys(111, "spawn ", argv[0]) ;
  }
  if (wait_pid(&wstat, pid) == -1) strerr_diefu1sys(111, "wait_pid") ;
  if (!flagnormalcrash && WIFSIGNALED(wstat))
  {
    char fmt[UINT_FMT] ;
    fmt[uint_fmt(fmt, WTERMSIG(wstat))] = 0 ;
    strerr_dief2x(1, "child crashed with signal ", fmt) ;
  }
  if (not == !wait_status(wstat)) return (int)e ;
  pathexec0_run(argv+argc1+1, envp) ;
  strerr_dieexec(111, argv[argc1+1]) ;
}
