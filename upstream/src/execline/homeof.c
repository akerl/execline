/* ISC license. */

#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include "buffer.h"
#include "strerr2.h"

#define USAGE "homeof user"

int main (int argc, char const *const *argv)
{
  struct passwd *pw ;
  PROG = "homeof" ;
  if (argc < 2) strerr_dieusage(100, USAGE) ;
  pw = getpwnam(argv[1]) ;
  if (!pw)
  {
    if (errno)
      strerr_diefu2sys(111, "get passwd entry for ", argv[1]) ;
    else
      strerr_diefu3x(111, "get passwd entry for ", argv[1], ": no such user") ;
  }
  if ((buffer_putsalign(buffer_1small, pw->pw_dir) == -1)
   || (buffer_putflush(buffer_1small, "\n", 1) == -1))
    strerr_diefu1sys(111, "write to stdout") ;
  return 0 ;
}
