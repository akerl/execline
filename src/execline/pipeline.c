/* ISC license. */

#include <unistd.h>
#include "sgetopt.h"
#include "fmtscan.h"
#include "strerr2.h"
#include "env.h"
#include "djbunix.h"
#include "execline.h"

#define USAGE "pipeline [ -d ] [ -r | -w ] { command... } command..."
#define dieusage() strerr_dieusage(100, USAGE)

int main (int argc, char const **argv, char const *const *envp)
{
  int df = 0, w = 0 ;
  PROG = "pipeline" ;
  {
    subgetopt_t l = SUBGETOPT_ZERO ;
    for (;;)
    {
      register int opt = subgetopt_r(argc, argv, "drw", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'd' : df = 1 ; break ;
        case 'r' : w = 0 ; break ;
        case 'w' : w = 1 ; break ;
        default : dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  {
    int fd[2] ;
    int pid ;
    int argc1 = el_semicolon(argv) ;
    if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;
    if (argc1 + 1 == argc) strerr_dief1x(100, "empty remainder") ;
    if (pipe(fd) == -1) strerr_diefu1sys(111, "create pipe") ;
    pid = df ? doublefork() : fork() ;
    switch (pid)
    {
      case -1: strerr_diefu2sys(111, df ? "double" : "", "fork") ;
      case 0:
        PROG = "pipeline (child)" ;
        fd_close(fd[w]) ;
        if (fd_move(!w, fd[!w]) < 0) strerr_diefu1sys(111, "fd_move") ;
        argv[argc1] = 0 ;
        pathexec0(argv) ;
        strerr_dieexec(111, argv[0]) ;
    }
    fd_close(fd[!w]) ;
    if (fd_move(w, fd[w]) < 0) strerr_diefu1sys(111, "fd_move") ;
    {
      char fmt[LASTPID_NAMELEN + 1 + UINT_FMT] = LASTPID_NAME "=" ;
      register unsigned int i = LASTPID_NAMELEN + 1 ;
      i += uint_fmt(fmt+i, pid) ; fmt[i++] = 0 ;
      pathexec_r(argv + argc1 + 1, envp, env_len(envp), fmt, i) ;
    }
    strerr_dieexec(111, argv[argc1 + 1]) ;
  }
}
