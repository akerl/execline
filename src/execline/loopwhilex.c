/* ISC license. */

#include <unistd.h>
#include "sgetopt.h"
#include "strerr2.h"
#include "fmtscan.h"
#include "djbunix.h"

#define USAGE "loopwhilex [ -n ] [ -x exitcode ] prog..."
#define dieusage() strerr_dieusage(100, USAGE)

int main (int argc, char const *const *argv, char const *const *envp)
{
  int wstat ;
  int not = 0, cont = 1 ;
  unsigned short e = 0 ;
  PROG = "loopwhilex" ;
  {
    subgetopt_t l = SUBGETOPT_ZERO ;
    for (;;)
    {
      register int opt = subgetopt_r(argc, argv, "nx:", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'n' : not = 1 ; break ;
        case 'x' : if (!ushort_scan(l.arg, &e)) dieusage() ; break ;
        default : dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  if (!argc) dieusage() ;

  while (cont)
  {
    int pid = fork() ;
    switch (pid)
    {
      case -1: strerr_diefu1sys(111, "fork") ;
      case 0:
      {
        pathexec_run(argv[0], argv, envp) ;
        strerr_diewu2sys(111, "spawn ", argv[0]) ;
      }
    }
    if (wait_pid(&wstat, pid) == -1) strerr_diefu1sys(111, "wait_pid") ;
    cont = not ^ (wait_status(wstat) == (int)e) ;
  }
  return wait_crashed(wstat) ? wait_stopsig(wstat) : 0 ;
}
