/* ISC license. */

#include "env.h"
#include "strerr2.h"
#include "skamisc.h"
#include "exlsn.h"

void el_substandrun (int argc, char const *const *argv, char const *const *envp, exlsn_t_ref info)
{
  satmp.len = 0 ;
  if (!env_string(&satmp, argv, (unsigned int)argc)) strerr_diefu1sys(111, "env_string") ;
  el_substandrun_str(&satmp, 0, envp, info) ;
}
