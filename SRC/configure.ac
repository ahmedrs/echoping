dnl $Id$

dnl Process this file with autoconf to produce a configure script.
AC_INIT(echoping.h)
AC_CANONICAL_HOST
AC_PROG_CC(cc gcc)
AM_INIT_AUTOMAKE(echoping, 5.2.0-BETA)
AM_CONFIG_HEADER(config.h)

dnl User options
AC_ARG_ENABLE(http,
 [--enable-http   HTTP (Web's main protocol) support],dnl
 [if test "$enableval" = "yes"; then
 AC_DEFINE(HTTP,,[HTTP support])
 HTTP=1
 fi],
 dnl Default: enable it
 [AC_DEFINE(HTTP,,[HTTP support])
  HTTP=1])
AC_ARG_ENABLE(icp,
 [--enable-icp   ICP (for testing Web proxies/caches) support],dnl
 [if test "$enableval" = "yes"; then
 AC_DEFINE(ICP,,[ICP support])
 ICP=1
 fi])
AC_ARG_ENABLE(smtp,
 [--enable-smtp   SMTP (Mail's main protocol) support],dnl
 [if test "$enableval" = "yes"; then
 AC_DEFINE(SMTP,,[SMTP support])
 SMTP=1
 fi],
 dnl Default: enable it
 [AC_DEFINE(SMTP,,[SMTP support])
  SMTP=1])
dnl IDN
dnl Default: enable it	
LIBIDN=1
AC_ARG_WITH(libidn,
 [--with-libidn[=DIR]   Internationalized Domain Names support (needs GNU libidn)],dnl
 [if test "$withval" != "no"; then
   if test "$withval" != "yes"; then
	IDNROOT=$withval
	LDFLAGS="${LDFLAGS} -L$IDNROOT/lib"
        CPPFLAGS="${CPPFLAGS} -I$IDNROOT/include"  
   fi
 else
   LIBIDN=0
 fi],
)
if test "$LIBIDN" = "1"; then
AC_DEFINE(LIBIDN,,[Internationalized Domain Names support])
fi
AC_ARG_WITH(ssl,
 [--with-ssl[=DIR]   SSL crypt support (needs OpenSSL)],dnl
 [if test "$withval" != "no"; then
   AC_DEFINE(OPENSSL,,[Crypto (SSL) support])
   OPENSSL=1
   if test "$withval" != "yes"; then
	SSLROOT=$withval
	LDFLAGS="${LDFLAGS} -L$SSLROOT/lib"
        CPPFLAGS="${CPPFLAGS} -I$SSLROOT/include"  
   fi
 fi],
 dnl Default: disable it
)
AC_ARG_WITH(gnutls,
 [--with-gnutls[=DIR]   SSL/TLS crypt support (needs GNU TLS), the argument DIR should not be necessary],dnl
 [if test "$withval" != "no"; then
   AC_DEFINE(GNUTLS,,[Crypto (TLS) support])
   GNUTLS=1
   CPPFLAGS="${CPPFLAGS} `libgnutls-config --cflags`"
   LDFLAGS="${LDFLAGS} `libgnutls-config --libs`"
   if test "$withval" != "yes"; then
	GNUTLSROOT=$withval
	LDFLAGS="${LDFLAGS} -L$GNUTLSROOT/lib"
        CPPFLAGS="${CPPFLAGS} -I$GNUTLSROOT/include"  
   fi
 fi],
 dnl Default: disable it
)

dnl See T/TCP later 

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/time.h unistd.h)

case $host_os in
osf*)
	# Stupid bug appeared in Tru64-OSF1 v5. socklen_t is undefined without
	# the following workaround. 
        CPPFLAGS="$CPPFLAGS -D_POSIX_PII_SOCKET"
        ;;
*darwin*)
 	# See bug #748145 and #765777
        CPPFLAGS="$CPPFLAGS -D_BSD_SOCKLEN_T_=int -no-cpp-precomp"
	;;
esac

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl Checks for library functions.
CF_LIB_SOCKET
CF_LIB_NSL
CF_LIB_MATH
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
dnl Some Unices like Tru64 or Mac OS X has getaddrinfo() or
dnl getnameinfo() but has it renamed in libc as something else so we
dnl must include <netdb.h> to get the redefinition. (Stolen from rsync)
dnl autoconf AC_CHECK_FUNCS does not allow headers to be easily included :-(
AC_CHECK_FUNCS(getaddrinfo getnameinfo inet_ntop, ,
	[AC_MSG_CHECKING([$ac_func again by including <netdb.h>])
	AC_TRY_LINK([#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>],[$ac_func(NULL, NULL, NULL, NULL);],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_ERROR([Missing mandatory function - echoping now uses the new network functions (RFC 2133) which are mandatory for IPv6])]
         	)])
		
AC_CHECK_FUNCS(gettimeofday socket sigaction strerror, , AC_MSG_ERROR(Missing mandatory function))
AC_CHECK_FUNCS(usleep)

if test "$LIBIDN" = "1"; then
CF_LIB_LIBIDN
fi
if test "$OPENSSL" = "1" && test "$GNUTLS" = "1"; then
AC_MSG_ERROR([Choose OpenSSL or GNU TLS but not both])
fi
if test "$OPENSSL" = "1"; then
CF_LIB_OPENSSL
fi
if test "$GNUTLS" = "1"; then
CF_LIB_GNUTLS
fi

dnl T/TCP
AC_MSG_CHECKING([T/TCP])
AC_TRY_COMPILE(
 [#include <sys/types.h>
  #include <sys/socket.h>
 ],
 [int foobar = MSG_EOF;],
 [AC_DEFINE(TTCP,,[Transaction TCP support])
  ac_have_ttcp="yes"],
 ac_have_ttcp=no)
AC_MSG_RESULT($ac_have_ttcp)
AC_ARG_ENABLE(ttcp,
 [--enable-ttcp   T/TCP (Transaction TCP) support],
 [if test "$enableval" = "yes"; then
   if test $ac_have_ttcp = "yes"; then
     AC_DEFINE(TTCP,,[Transaction TCP support])
     TTCP=1
   else
     AC_MSG_WARN([No T/TCP support on this system, request ignored])
   fi
  fi],
 dnl Default: enable it if supported
 if test $ac_have_ttcp = "yes"; then
   AC_DEFINE(TTCP,,[Transaction TCP support])
   TTCP=1
 fi)

dnl Type Of Service
AC_MSG_CHECKING([Type Of Service])
AC_TRY_COMPILE(
 [#include <sys/types.h>
  #include <netinet/in.h>
 ],
 [int foobar = IP_TOS;],
 [AC_DEFINE(HAVE_TOS,,[Type Of Service support])
  ac_have_tos="yes"],
 ac_have_tos="no")
AC_MSG_RESULT($ac_have_tos)
AC_ARG_ENABLE(tos,
 [--enable-tos   TOS (Type Of Service) support],
 [if test "$enableval" = "yes"; then
   if test $ac_have_tos = "yes"; then
     AC_DEFINE(HAVE_TOS,,[Type Of Service support])
     TOS=1
   else
     AC_MSG_WARN([No TOS support on this system, request ignored])
   fi
  fi],
 dnl Default: enable it if supported
 if test $ac_have_tos = "yes"; then
   AC_DEFINE(HAVE_TOS,,[Type Of Service support])
   TOS=1
 fi)

dnl Socket priority
dnl Linux only, it seems. Anyone knows a standard way to do so?
AC_MSG_CHECKING([Socket priority])
AC_TRY_COMPILE(
 [#include <sys/types.h>
  #include <sys/socket.h>
 ],
 [int foobar = SO_PRIORITY;],
 [AC_DEFINE(HAVE_SOCKET_PRIORITY)
  ac_have_priority="yes"],
 ac_have_priority="no")
AC_MSG_RESULT($ac_have_priority)
AC_ARG_ENABLE(priority,
 [--enable-priority   PRIORITY (socket priority) support],
 [if test "$enableval" = "yes"; then
   if test $ac_have_priority = "yes"; then
     AC_DEFINE(HAVE_SOCKET_PRIORITY,,[Socket priority support])
     PRIORITY=1
   else
     AC_MSG_WARN([No socket priority support on this system, request ignored])
   fi
  fi],
 dnl Default: enable it if supported
 if test $ac_have_priority = "yes"; then
   AC_DEFINE(HAVE_SOCKET_PRIORITY,,[Socket priority support])
   PRIORITY=1
 fi)

if test "$GCC" = yes; then
        CFLAGS="$CFLAGS -Wall"
fi

AC_OUTPUT(Makefile)

AC_DEFUN([DISPLAY_SETTING],
[
AC_MSG_CHECKING($1)
if [ eval 'test "$$1" = "1"' > /dev/null]; then
	AC_MSG_RESULT( enabled)
else 
	AC_MSG_RESULT( disabled)
fi
])dnl
echo ""
echo "Configuration of echoping:"

DISPLAY_SETTING(HTTP)
DISPLAY_SETTING(ICP)
DISPLAY_SETTING(OPENSSL)
DISPLAY_SETTING(GNUTLS)
DISPLAY_SETTING(SMTP)
DISPLAY_SETTING(LIBIDN)
DISPLAY_SETTING(TTCP)
DISPLAY_SETTING(TOS)
DISPLAY_SETTING(PRIORITY)




