/* ISC license. */

#include <unistd.h>
#include <errno.h>
#include "allreadwrite.h"
#include "sgetopt.h"
#include "bytestr.h"
#include "buffer.h"
#include "fmtscan.h"
#include "strerr2.h"
#include "stralloc.h"
#include "genalloc.h"
#include "env.h"
#include "djbunix.h"
#include "skamisc.h"
#include "netstring.h"
#include "execline-config.h"
#include "execline.h"

#ifdef EXECLINE_DOSUBST_COMPAT
# define USAGE "forbacktickx [ -E | -e ] [ -p | -x breakcode ] [ -n ] [ -C | -c ] [ -0 | -d delim ] var { backtickcmd... } command..."
# define OPTIONS "EepnCc0d:x:"
#else
# define USAGE "forbacktickx [ -p | -x breakcode ] [ -n ] [ -C | -c ] [ -0 | -d delim ] var { backtickcmd... } command..."
# define OPTIONS "epnCc0d:x:"
#endif
#define dieusage() strerr_dieusage(100, USAGE)


int main (int argc, char const **argv, char const *const *envp)
{
  genalloc pids = GENALLOC_ZERO ; /* array of unsigned ints */
  char const *delim = " \n\r\t" ;
  unsigned int delimlen = 4 ;
  char const *x ;
  int argc1 ;
  int breakcode = -2 ;
  int crunch = 0, chomp = 0 ;
#ifdef EXECLINE_DOSUBST_COMPAT
  int dosubst = 0 ;
#endif
  PROG = "forbacktickx" ;
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
        case 'p' :
        {
          if (!genalloc_ready(unsigned int, &pids, 2))
            strerr_diefu1sys(111, "genalloc_ready") ;
          break ;
        }
        case 'n' : chomp = 1 ; break ;
        case 'C' : crunch = 1 ; break ;
        case 'c' : crunch = 0 ; break ;
        case '0' : delim = "" ; delimlen = 1 ; break ;
        case 'd' : delim = l.arg ; delimlen = str_len(delim) ; break ;
        case 'x' :
        {
          unsigned short b ;
          if (!ushort_scan(l.arg, &b)) dieusage() ;
          breakcode = b ;
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
  if (!argc1) strerr_dief1x(100, "empty block") ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;
  {
    int pidw ;
    int p[2] ;
    if (pipe(p) < 0) strerr_diefu1sys(111, "create pipe") ;
    pidw = fork() ;
    switch (pidw)
    {
      case -1: strerr_diefu1sys(111, "fork") ;
      case 0:
      {
        PROG = "forbacktickx (backtick child)" ;
        argv[argc1] = 0 ;
        fd_close(p[0]) ;
        if (fd_move(1, p[1]) == -1) strerr_diefu1sys(111, "fd_move") ;
        pathexec_run(argv[0], argv, envp) ;
        strerr_diefu2sys(111, "spawn ", argv[0]) ;
      }
    }
    fd_close(p[1]) ;
    {
      char buf[BUFFER_INSIZE] ;
      buffer b = BUFFER_INIT(&fd_read, p[0], buf, BUFFER_INSIZE) ;
      stralloc modif = STRALLOC_ZERO ;
      unsigned int envlen = env_len(envp) ;
      unsigned int modifstart = str_len(x)+1 ;
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
      if (!stralloc_ready(&modif, modifstart+1))
        strerr_diefu1sys(111, "stralloc_ready") ;
      byte_copy(modif.s, modifstart-1, x) ;
      modif.s[modifstart-1] = '=' ;
      for (;;)
      {
        int pid ;
        int wstat ;
        modif.len = modifstart ;
        if (delimlen)
        {
          register int r = skagetlnsep(&b, &modif, delim, delimlen) ;
          if (!r) break ;
          else if (r < 0)
          {
            if (errno != EPIPE) strerr_diefu1sys(111, "skagetlnsep") ;
            if (chomp) break ;
          }
          else modif.len-- ;
          if ((modif.len == modifstart) && crunch) continue ;
        }
        else
        {
          unsigned int unread = 0 ;
          if (netstring_get(&b, &modif, &unread) <= 0)
          {
            if (netstring_okeof(&b, unread)) break ;
            else strerr_diefu1sys(111, "netstring_get") ;
          }
        }
        if (!stralloc_0(&modif)) strerr_diefu1sys(111, "stralloc_0") ;
        pid = fork() ;
        switch (pid)
        {
          case -1: strerr_diefu1sys(111, "fork") ;
          case 0:
          {
            PROG = "forbacktickx (loop child)" ;
#ifdef EXECLINE_DOSUBST_COMPAT
            pathexec_r(newargv, envp, envlen, modif.s, modif.len) ;
            strerr_dieexec(111, newargv[0]) ;
#else
            pathexec_r(argv + argc1 + 1, envp, envlen, modif.s, modif.len) ;
            strerr_dieexec(111, argv[argc1 + 1]) ;
#endif
          }
        }
        if (pids.s)
        {
          if (!genalloc_append(unsigned int, &pids, (unsigned int *)&pid))
            strerr_diefu1sys(111, "genalloc_append") ;
        }
        else
        {
          if (wait_pid(&wstat, pid) < 0)
#ifdef EXECLINE_DOSUBST_COMPAT
            strerr_diefu2sys(111, "wait for ", newargv[0]) ;
#else
            strerr_diefu2sys(111, "wait for ", argv[argc1 + 1]) ;
#endif
          if (wait_status(wstat) == breakcode) return breakcode ;
        }
      }
      stralloc_free(&modif) ;
    }
    fd_close(p[0]) ;
    if (!genalloc_append(unsigned int, &pids, (unsigned int *)&pidw))
      strerr_diefu1sys(111, "genalloc_append") ;
  }
  if (!waitn(genalloc_s(unsigned int, &pids), genalloc_len(unsigned int, &pids)))
    strerr_diefu1sys(111, "waitn") ;
  /* genalloc_free(unsigned int, &pids) ; */
  return 0 ;
}
