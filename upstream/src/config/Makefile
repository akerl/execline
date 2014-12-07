it: command include library library.so


execline-config.h: stupid execline-version.stupid execline-config-header execline-config-nosp execline-config-sp.stupid execline-config-footer
	{ \
	  cat execline-config-header ; \
	  ./stupid < execline-version.stupid ; \
	  if test -f flag-slashpackage ; then ./stupid < execline-config-sp.stupid ; else cat execline-config-nosp ; fi ; \
	  cat execline-config-footer ; \
	} > execline-config.h


command:

include: execline-config.h

library:

library.so:
