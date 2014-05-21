/* ISC license. */

#include "stralloc.h"
#include "exlsn.h"

void exlsn_free (exlsn_t_ref info)
{
  stralloc_free(&info->vars) ;
  stralloc_free(&info->values) ;
  stralloc_free(&info->data) ;
}

