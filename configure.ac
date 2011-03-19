AC_INIT([vARCH], [0.1], [ben.vighy@gmail.com])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([dist-bzip2 -Wall])

AC_PROG_CXX
AM_PROG_LEX
AC_PROG_YACC

# AM_CONFIG_HEADER([include/config.h])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/new-asm/Makefile
	src/trans-asm/Makefile
])

###############################################################################
# Debug builds
AC_ARG_WITH(messaging,
  AS_HELP_STRING([--with-messaging=level],
                 [enable various messaging levels, default: 1 (info)
                  Options: 3 (debug), 2 (warnings), 1 (info), 0 (no messages)]),
  [ case "${withval}" in
      3)     messaging=debug ;;
      2)     messaging=warn ;;
      1)     messaging=info ;;
      0)     messaging=nomessage ;;
      false) messaging=nomessage ;;
      no)    messaging=nomessage ;;
      *)   AC_MSG_ERROR([bad value ${withval} for --with-messaging]) ;;
    esac],
  [messaging=info])
AS_IF([test "x$messaging" == xdebug],
  [AC_DEFINE([DEBUG], [1],
        [Define if you want a debug build])
   ])
AS_IF([test "x$messaging" == xwarn],
  [AC_DEFINE([WARNING], [1],
        [Define if you want a build with warning information])
   ])
AS_IF([test "x$messaging" == xinfo],
  [AC_DEFINE([INFO], [1],
        [Define if you want a build with information])
   ])
AS_IF([test "x$messaging" != xdebug],
      [ AC_SUBST([CFLAGS],["${CFLAGS} -fomit-frame-pointer -fforce-addr"])
        AC_SUBST([CXXFLAGS],["${CXXFLAGS} -fomit-frame-pointer -fforce-addr"])
       ])
###############################################################################

AC_OUTPUT

dnl
dnl Output some configuration info for the user
dnl
echo ""
echo "  Configuration summary:"
echo ""
echo "        prefix:          $prefix"
echo "        exec_prefix:     $exec_prefix"
echo "        libdir:          $libdir"
echo "        includedir:      $includedir"

dnl
dnl Output the features included
dnl
echo ""
echo "        CFLAGS:          $CFLAGS"
echo "        CXXFLAGS:        $CXXFLAGS"
echo ""
echo "        Verbosity level: $messaging"

echo ""
echo ""
