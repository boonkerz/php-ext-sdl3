PHP_ARG_WITH(sdl3, [for sdl3 support], [
AS_HELP_STRING([--with-sdl3[=DIR]], [Enable sdl3 support. DIR is the prefix for SDL3 installation.])
])

PHP_ARG_WITH(sdl3_image, [for sdl3_image support], [
AS_HELP_STRING([--with-sdl3-image[=DIR]], [Enable sdl3_image support. DIR is the prefix for SDL3_image installation.])
])

PHP_ARG_WITH(sdl3_ttf, [for sdl3_ttf support], [
AS_HELP_STRING([--with-sdl3-ttf[=DIR]], [Enable sdl3_ttf support. DIR is the prefix for SDL3_ttf installation.])
])

if test "$PHP_SDL3" != "no"; then
  if test -d "$PHP_SDL3"; then
    PKG_CONFIG_PATH="$PHP_SDL3/lib/pkgconfig:$PHP_SDL3/share/pkgconfig:$PKG_CONFIG_PATH"
  fi

  PKG_CHECK_MODULES([SDL3], [sdl3 >= 3.0.0], [
    CFLAGS="$CFLAGS $SDL3_CFLAGS"
  ],[
    AC_MSG_ERROR([SDL3 not found. Please check your installation or use --with-sdl3=/path/to/sdl3])
  ])

  dnl Prefer static SDL3 libs if available
  AC_MSG_CHECKING([for static SDL3 libs])
  SDL3_STATIC_LIBS=`$PKG_CONFIG --libs --static sdl3 2>/dev/null`
  if test "x$SDL3_STATIC_LIBS" != "x"; then
    AC_MSG_RESULT([$SDL3_STATIC_LIBS])
    LDFLAGS="$LDFLAGS $SDL3_STATIC_LIBS"
  else
    AC_MSG_RESULT([not found, using shared SDL3 libs])
    LDFLAGS="$LDFLAGS $SDL3_LIBS"
  fi

  if test "$PHP_SDL3_IMAGE" != "no"; then
    if test -d "$PHP_SDL3_IMAGE"; then
        PKG_CONFIG_PATH="$PHP_SDL3_IMAGE/lib/pkgconfig:$PHP_SDL3_IMAGE/share/pkgconfig:$PKG_CONFIG_PATH"
    fi

    PKG_CHECK_MODULES([SDL3_IMAGE], [sdl3-image >= 3.0.0], [
      CFLAGS="$CFLAGS $SDL3_IMAGE_CFLAGS"
    ],[
      AC_MSG_ERROR([SDL3_image not found. Please check your installation or use --with-sdl3-image=/path/to/sdl3_image])
    ])

    dnl Prefer static SDL3_image libs if available
    AC_MSG_CHECKING([for static SDL3_image libs])
    SDL3_IMAGE_STATIC_LIBS=`$PKG_CONFIG --libs --static sdl3-image 2>/dev/null`
    if test "x$SDL3_IMAGE_STATIC_LIBS" != "x"; then
      AC_MSG_RESULT([$SDL3_IMAGE_STATIC_LIBS])
      LDFLAGS="$LDFLAGS $SDL3_IMAGE_STATIC_LIBS"
    else
      AC_MSG_RESULT([not found, using shared SDL3_image libs])
      LDFLAGS="$LDFLAGS $SDL3_IMAGE_LIBS"
    fi
  fi

  if test "$PHP_SDL3_TTF" != "no"; then
    if test -d "$PHP_SDL3_TTF"; then
        PKG_CONFIG_PATH="$PHP_SDL3_TTF/lib/pkgconfig:$PHP_SDL3_TTF/share/pkgconfig:$PKG_CONFIG_PATH"
    fi

    PKG_CHECK_MODULES([SDL3_TTF], [sdl3-ttf >= 3.0.0], [
      CFLAGS="$CFLAGS $SDL3_TTF_CFLAGS"
    ],[
      AC_MSG_ERROR([SDL3_ttf not found. Please check your installation or use --with-sdl3-ttf=/path/to/sdl3_ttf])
    ])

    dnl Prefer static SDL3_ttf libs if available
    AC_MSG_CHECKING([for static SDL3_ttf libs])
    SDL3_TTF_STATIC_LIBS=`$PKG_CONFIG --libs --static sdl3-ttf 2>/dev/null`
    if test "x$SDL3_TTF_STATIC_LIBS" != "x"; then
      AC_MSG_RESULT([$SDL3_TTF_STATIC_LIBS])
      LDFLAGS="$LDFLAGS $SDL3_TTF_STATIC_LIBS"
    else
      AC_MSG_RESULT([not found, using shared SDL3_ttf libs])
      LDFLAGS="$LDFLAGS $SDL3_TTF_LIBS"
    fi
  fi

  dnl Optional libnotify support for desktop notifications
  PKG_CHECK_MODULES([LIBNOTIFY], [libnotify], [
    AC_DEFINE([HAVE_LIBNOTIFY], [1], [Enable libnotify for desktop_notify()])
    CFLAGS="$CFLAGS $LIBNOTIFY_CFLAGS"
    LDFLAGS="$LDFLAGS $LIBNOTIFY_LIBS"
  ], [
    AC_MSG_WARN([libnotify not found via pkg-config, desktop_notify() will be disabled])
  ])

  dnl Optional AppIndicator support for tray (Linux)
  PKG_CHECK_MODULES([APPINDICATOR], [appindicator3-0.1], [
    AC_DEFINE([HAVE_TRAY_APPINDICATOR], [1], [Enable tray AppIndicator backend])
    CFLAGS="$CFLAGS $APPINDICATOR_CFLAGS"
    LDFLAGS="$LDFLAGS $APPINDICATOR_LIBS"
  ], [
    AC_MSG_WARN([appindicator3-0.1 not found, tray AppIndicator backend will be disabled])
  ])

  SDL_SOURCE_FILES="sdl3.c helper.c sdl3_image.c sdl3_ttf.c sdl3_events.c"

  PHP_NEW_EXTENSION(sdl3, $SDL_SOURCE_FILES, $ext_shared)
fi
