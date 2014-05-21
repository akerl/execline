/* ISC license. */

#include <unistd.h>
#include "fmtscan.h"
#include "strerr2.h"
#include "env.h"
#include "djbunix.h"
#include "execline.h"

void el_execsequence (char const *const *argv1, char const *const *argv2, char const *const *envp)
{
  if (!argv2[0])
  {
    pathexec0_run(argv1, envp) ;
    strerr_dieexec(111, argv1[0]) ;
  }
  {
    int pid = fork() ;
    switch (pid)
    {
      case -1: strerr_diefu1sys(111, "fork") ;
      case 0:
        pathexec0_run(argv1, envp) ;
        strerr_diewu2sys(111, "spawn ", argv1[0]) ;
    }
    {
      int wstat ;
      unsigned int j = LASTEXITCODE_NAMELEN + 1 ;
      char fmt[LASTEXITCODE_NAMELEN + 1 + UINT_FMT] = LASTEXITCODE_NAME "=" ;
      if (wait_pid(&wstat, pid) == -1)
        strerr_diefu2sys(111, "wait for ", argv1[0]) ;
      j += uint_fmt(fmt + j, wait_status(wstat)) ; fmt[j++] = 0 ;
      pathexec_r(argv2, envp, env_len(envp), fmt, j) ;
    }
  }
  strerr_dieexec(111, argv2[0]) ;
}
