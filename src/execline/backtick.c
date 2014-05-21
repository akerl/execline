/* ISC license. */

#include <unistd.h>
#include "bytestr.h"
#include "sgetopt.h"
#include "strerr2.h"
#include "stralloc.h"
#include "djbunix.h"
#include "execline-config.h"
#include "execline.h"

#ifdef EXECLINE_DOSUBST_COMPAT
# define USAGE "backtick [ -E | -e ] [ -i ] [ -n ] [ -s ] [ -C | -c ] [ -d delim ] var { prog... } remainder..."
# define OPTIONS "EeinsCcd:"
#else
# define USAGE "backtick [ -i ] [ -n ] var { prog... } remainder..."
# define OPTIONS "ein"
#endif

#define dieusage() strerr_dieusage(100, USAGE)

int main (int argc, char const **argv, char const *const *envp)
{
  subgetopt_t localopt = SUBGETOPT_ZERO ;
  int argc1 ;
  stralloc modif = STRALLOC_ZERO ;
  int insist = 0, chomp = 0 ;
#ifdef EXECLINE_DOSUBST_COMPAT
  int dosubst = 0 ;
  char const *x ;
  int dosplit = 0, docrunch = 0 ;
  char const *delim = 0 ;
#endif
  PROG = "backtick" ;
  for (;;)
  {
    register int opt = subgetopt_r(argc, argv, OPTIONS, &localopt) ;
    if (opt < 0) break ;
    switch (opt)
    {
      case 'i' : insist = 1 ; break ;
      case 'n' : chomp = 1 ; break ;
#ifdef EXECLINE_DOSUBST_COMPAT
      case 'e' : dosubst = 0 ; break ;
      case 'E' : dosubst = 1 ; break ;
      case 's' : dosplit = 1 ; break ;
      case 'C' : docrunch = 1 ; break ;
      case 'c' : docrunch = 0 ; break ;
      case 'd' : delim = localopt.arg ; break ;
#else
      case 'e' : break ; /* compat */
#endif
      default : dieusage() ;
    }
  }
  argc -= localopt.ind ; argv += localopt.ind ;

  if (argc < 2) dieusage() ;
  if (!*argv[0]) strerr_dief1x(100, "empty variable not accepted") ;
#ifdef EXECLINE_DOSUBST_COMPAT
  x = argv[0] ;
#endif
  if (!stralloc_cats(&modif, argv[0]) || !stralloc_catb(&modif, "=", 1))
    strerr_diefu1sys(111, "stralloc_catb") ;
  argc-- ; argv++ ;
  argc1 = el_semicolon(argv) ;
  if (!argc1) strerr_dief1x(100, "empty block") ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;

  {
    int p[2] ;
    int pid ;
    if (pipe(p) == -1) strerr_diefu1sys(111, "pipe") ;
    pid = fork() ;
    switch (pid)
    {
      case -1: strerr_diefu1sys(111, "fork") ;
      case 0:
        argv[argc1] = 0 ;
        fd_close(p[0]) ;
        PROG = "backtick (child)" ;
        if (fd_move(1, p[1]) == -1) strerr_diefu1sys(111, "fd_move") ;
        pathexec_run(argv[0], argv, envp) ;
        strerr_dieexec(111, argv[0]) ;
    }
    fd_close(p[1]) ;
    if (!slurp(&modif, p[0])) strerr_diefu1sys(111, "slurp") ;
    fd_close(p[0]) ;
    if (wait_pid(&p[0], pid) == -1) strerr_diefu1sys(111, "wait_pid") ;
    if (insist && wait_status(p[0]))
      strerr_dief1x(wait_status(p[0]), "child process exited non-zero") ;
  }
  if (argc == argc1 - 1) return 0 ;
  if (!stralloc_0(&modif)) strerr_diefu1sys(111, "stralloc_catb") ;

#ifdef EXECLINE_DOSUBST_COMPAT
  {
    char const *const *newargv = argv + argc1 + 1 ;
    char const *compatargv[dosubst ? argc - argc1 + 4 + chomp + dosplit + docrunch + 2 * !!delim : 1] ;
    if (dosubst)
    {
      register unsigned int m = 0 ;
      register unsigned int i = 0 ;
      compatargv[m++] = EXECLINE_BINPREFIX "import" ;
      if (chomp) compatargv[m++] = "-n" ;
      if (dosplit) compatargv[m++] = "-s" ;
      if (docrunch) compatargv[m++] = "-C" ;
      if (delim)
      {
        compatargv[m++] = "-d" ;
        compatargv[m++] = delim ;
      }
      compatargv[m++] = x ;
      compatargv[m++] = EXECLINE_BINPREFIX "unexport" ;
      compatargv[m++] = x ;
      for (; (int)i < argc - argc1 ; i++) compatargv[m + i] = argv[argc1 + 1 + i] ;
      newargv = compatargv ;
    }
    else
#endif
    {
      unsigned int reallen = str_len(modif.s) ;
      if (reallen < modif.len - 1)
      {
        if (insist)
          strerr_dief1x(1, "child process output contained a null character") ;
        else
          modif.len = reallen + 1 ;
      }
      if (chomp && (modif.s[modif.len - 2] == '\n'))
        modif.s[--modif.len - 1] = 0 ;
    }

#ifdef EXECLINE_DOSUBST_COMPAT
    pathexec_r(newargv, envp, env_len(envp), modif.s, modif.len) ;
    strerr_dieexec(111, newargv[0]) ;
  }
#else
  pathexec_r(argv + argc1 + 1, envp, env_len(envp), modif.s, modif.len) ;
  strerr_dieexec(111, argv[argc1 + 1]) ;
#endif
}
