dnl $Id$
dnl config.m4 for extension php_tracer

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(php_tracer, for php_tracer support,
dnl Make sure that the comment is aligned:
[  --with-php_tracer             Include php_tracer support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(php_tracer, whether to enable php_tracer support,
dnl Make sure that the comment is aligned:
dnl [  --enable-php_tracer           Enable php_tracer support])

if test "$PHP_PHP_TRACER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-php_tracer -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/php_tracer.h"  # you most likely want to change this
  dnl if test -r $PHP_PHP_TRACER/$SEARCH_FOR; then # path given as parameter
  dnl   PHP_TRACER_DIR=$PHP_PHP_TRACER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for php_tracer files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PHP_TRACER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PHP_TRACER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the php_tracer distribution])
  dnl fi

  dnl # --with-php_tracer -> add include path
  dnl PHP_ADD_INCLUDE($PHP_TRACER_DIR/include)

  dnl # --with-php_tracer -> check for lib and symbol presence
  dnl LIBNAME=php_tracer # you may want to change this
  dnl LIBSYMBOL=php_tracer # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PHP_TRACER_DIR/lib, PHP_TRACER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PHP_TRACERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong php_tracer lib version or lib not found])
  dnl ],[
  dnl   -L$PHP_TRACER_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PHP_TRACER_SHARED_LIBADD)

  dnl PHP_NEW_EXTENSION(php_tracer, php_tracer.c slog.c, $ext_shared)

LIBOTHER_LIBNAME="glib-2.0"
dnl  LIBSYMBOL="glib-2.0"
dnl  LIBOTHER_LIBS="`pkg-config --cflags --libs glib-2.0`"
dnl  LIBOTHER_INCDIRS="`pkg-config --cflags glib-2.0`"
dnl  PHP_CHECK_LIBRARY($LIBOTHER_LIBNAME,$LIBSYMBOL,
dnl  [
dnl	  PHP_EVAL_INCLUDE(`pkg-config --cflags glib-2.0`)
          PHP_ADD_INCLUDE(`pkg-config --cflags --libs glib-2.0`)
	  PHP_EVAL_LIBLINE(`pkg-config --cflags glib-2.0`,PHP_TRACER_SHARED_LIBADD)
	  PHP_ADD_LIBRARY($LIBOTHER_LIBNAME,1,PHP_TRACER_SHARED_LIBADD)
	  PHP_SUBST(PHP_TRACER_SHARED_LIBADD)
dnl  ],[AC_MSG_ERROR([wrong lib$LIBNAME version or library not found])
dnl  ])
  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(stdc++,"",EXTRA_LDFLAGS)
  CPPFILE="php_tracer.cpp slog.cpp"
  PHP_NEW_EXTENSION(php_tracer, $CPPFILE,$ext_shared)    

fi
