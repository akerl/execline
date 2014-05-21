/* ISC license. */

#include "strerr2.h"
#include "exlsn.h"

#define USAGE "import [ -i | -D default ] [ -n ] [ -s ] [ -C | -c ] [ -d delim ] var prog..."

int main (int argc, char const **argv, char const *const *envp)
{
  PROG = "import" ;
  exlsn_main(argc, argv, envp, &exlsn_import, USAGE) ;
}
