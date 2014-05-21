/* ISC license. */

#include <errno.h>
#include "uint.h"
#include "bytestr.h"
#include "stralloc.h"
#include "execline.h"

int el_popenv (stralloc *sa, char const *const *envp, unsigned int envlen, char const *const *list, unsigned int listlen)
{
  unsigned int i = 0, salen = sa->len, count = 0 ;
  for (; i < envlen ; i++)
  {
    unsigned int equal, colon, n ;
    unsigned int j = 0 ;
    for (; j < listlen ; j++) if (str_start(envp[i], list[j])) break ;
    if (j == listlen) goto copyit ;
    j = str_len(list[j]) ;
    colon = j + str_chr(envp[i] + j, ':') ;
    equal = j + str_chr(envp[i] + j, '=') ;
    if (!envp[i][equal]) goto badenv ;
    if (colon >= equal) { count++ ; continue ; }
    if (colon + 1 + uint_scan(envp[i] + colon + 1, &n) != equal) goto copyit ;
    if (!n) goto copyit ;
    if (!stralloc_catb(sa, envp[i], colon)) goto err ;
    if (n > 1)
    {
      char fmt[UINT_FMT+1] = ":" ;
      n = 1 + uint_fmt(fmt+1, n-1) ;
      if (!stralloc_catb(sa, fmt, n)) goto err ;
    }
    if (!stralloc_cats(sa, envp[i] + equal) || !stralloc_0(sa)) goto err ;
    continue ;
copyit:
    if (!stralloc_cats(sa, envp[i]) || !stralloc_0(sa)) goto err ;
  }
  return (int)count ;

badenv :
  errno = EINVAL ;
err:
  sa->len = salen ;
  return -1 ;
}
