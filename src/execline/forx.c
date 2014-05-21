/* ISC license. */

#include <unistd.h>
#include "sgetopt.h"
#include "bytestr.h"
#include "fmtscan.h"
#include "strerr2.h"
#include "env.h"
#include "djbunix.h"
#include "skamisc.h"
#include "execline-config.h"
#include "execline.h"

#ifdef EXECLINE_DOSUBST_COMPAT
# define USAGE "forx [ -E | -e ] [ -p | -x breakcode ] var { values... } command..."
# define OPTIONS "Eepx:"
#else
# define USAGE "forx [ -p | -x breakcode ] var { values... } command..."
# define OPTIONS "epx:"
#endif

#define dieusage() strerr_dieusage(100, USAGE)

int main (int argc, char const **argv, char const *const *envp)
{
  char const *x ;
  int argc1 ;
  int breakcode = -2 ;
  int flagpar = 0 ;
#ifdef EXECLINE_DOSUBST_COMPAT
  int dosubst = 0 ;
#endif
  PROG = "forx" ;
  {
    subgetopt_t l = SUBGETOPT_ZERO ;
    for (;;)
    {
      register int opt = subgetopt_r(argc, argv, OPTIONS, &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
#ifdef EXECLINE_DOSUBST_COMPAT
	case 'E' : dosubst = 1 ; break ;
	case 'e' : dosubst = 0 ; break ;
#else
        case 'e' : break ; /* compat */
#endif
        case 'p' : flagpar = 1 ; break ;
        case 'x' :
        {
          unsigned short b ;
          if (!ushort_scan(l.arg, &b)) dieusage() ;
          breakcode = (int)b ;
          break ;
        }
        default : dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }

  if (argc < 2) dieusage() ;
  x = argv[0] ; if (!*x) dieusage() ;
  argv++ ; argc-- ;
  argc1 = el_semicolon(argv) ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;
  if (!argc1 || (argc1 + 1 == argc)) return 0 ;
  {
    unsigned int pids[flagpar ? argc1 : 1] ;
    unsigned int envlen = env_len(envp) ;
    unsigned int varlen = str_len(x) ;
    unsigned int i = 0 ;
#ifdef EXECLINE_DOSUBST_COMPAT
    char const *const *newargv = argv + argc1 + 1 ;
    char const *compatargv[dosubst ? argc - argc1 + 4 : 1] ;
    if (dosubst)
    {
      register unsigned int i = 0 ;
      compatargv[0] = EXECLINE_BINPREFIX "import" ;
      compatargv[1] = x ;
      compatargv[2] = EXECLINE_BINPREFIX "unexport" ;
      compatargv[3] = x ;
      for (; (int)i < argc - argc1 ; i++) compatargv[4 + i] = argv[argc1 + 1 + i] ;
      newargv = compatargv ;
    }
#endif
                                                                                      
    for (; i < (unsigned int)argc1 ; i++)
    {
      int pid = fork() ;
      switch (pid)
      {
        case -1: strerr_diefu1sys(111, "fork") ;
        case 0:
        {
          unsigned int vallen = str_len(argv[i]) ;
          char modif[varlen + vallen + 2] ;
          byte_copy(modif, varlen, x) ;
          modif[varlen] = '=' ;
          byte_copy(modif + varlen + 1, vallen, argv[i]) ;
          modif[varlen + vallen + 1] = 0 ;
          PROG = "forx (child)" ;
#ifdef EXECLINE_DOSUBST_COMPAT
          pathexec_r(newargv, envp, envlen, modif, varlen + vallen + 2) ;
          strerr_dieexec(111, newargv[0]) ;
#else
          pathexec_r(argv + argc1 + 1, envp, envlen, modif, varlen + vallen + 2) ;
          strerr_dieexec(111, argv[argc1 + 1]) ;
#endif
        }
      }
      if (flagpar) pids[i] = pid ;
      else
      {
        int wstat ;
        if (wait_pid(&wstat, pid) == -1)
          strerr_diefu2sys(111, "wait for ", argv[argc1+1]) ;
        if (wait_status(wstat) == breakcode) return breakcode ;
      }
    }
    if (flagpar)
      if (!waitn(pids, argc1)) strerr_diefu1sys(111, "waitn") ;
  }
  return 0 ;
}
