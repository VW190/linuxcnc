#                                               -*- Autoconf -*-
# Procesar este archivo con autoconf para generar un script de configuración.
#
# Descripción: configure.ac
# Autonconf genera un ./configure a partir de este archivo
# que se utiliza para descubrir varios programas/dependencias
# uso: autoconf
#
# Authors: paul_c alex_joni jepler
# License: GPL Version 2
# System: Linux
#
# Copyright (c) 2004 All rights reserved.
#
# Last change:
# This file is released under GPL, refer to docs/README for details
#
# This file is adapted to replace the old ./configure script
# therefor it includes parts of that script

##############################################################################
# Section 1                                                                  #
# Inicialización Autoconf                                                    #
##############################################################################

# versión de autoconf minima
AC_PREREQ(2.69)
# Inicializacion. AC_INIT (package, version, [bug-report], [tarname], [url])
AC_INIT([LinuxCNC],[m4_normalize(esyscmd(cat ../VERSION))],[emc-developers@lists.sourceforge.net])
# Archivo unico en srcdir, para chequeo de la variable
AC_CONFIG_SRCDIR(emc/motion/motion.c)
# test y accion en caso de directorio distinto a /src
if test "$srcdir" != "."; then
    AC_MSG_ERROR([No se admite la construcción fuera de srcdir])
fi

# Determinar el compilador C++ a usar de la lista
AC_PROG_CXX([$RTSCC c++ clang++])
# Si se usa AC_PROG_INSTALL, se debe incluir install-sh en la distribución.
# En este paquete de fuentes, autogen.sh cuida de la presencia de install-sh
#   que viene con la instalación de autoconf
AC_PROG_INSTALL

# m4_include advierte de inclusion multiple de archivos
# Ver https://www.gnu.org/software/autoconf-archive/The-Macros.html
# Helper sencillo para garantizar que se hayan definido otras macros y estén disponibles para su uso.
m4_include([m4/ax_require_defined.m4])
# check de cobertura del lenguaje de referencia en el compilador para la versión especificada del estándar C++.
m4_include([m4/ax_cxx_compile_stdcxx.m4])
# check de cobertura del lenguaje en el compilador para la versión estándar C++17.
m4_include([m4/ax_cxx_compile_stdcxx_17.m4])
# Comprobación completa del entorno de desarrollo de Python.
m4_include([m4/ax_python.m4])
# Comprueba Python e intenta obtener la ruta include a 'Python.h'
m4_include([m4/ax_python_devel.m4])
# Test para librerias Boost C++ de version particular
m4_include([m4/ax_boost_base.m4])
# Comprueba si la biblioteca Boost.Python está instalada.
m4_include([m4/ax_boost_python.m4])
# haz esto opcional - probablemente solo resultará en warnings
AX_CXX_COMPILE_STDCXX_17(ext,mandatory)

# Notificar verificando build toplevel, directorio padre de /src
AC_MSG_CHECKING(directorio superior)
BUILD_TOPLEVEL="$(cd ..; pwd -P)"
AC_MSG_RESULT($BUILD_TOPLEVEL)

# Crear header con directivas #define
AC_CONFIG_HEADERS([config.h])

# Support some of the standard configure options for directories
# prefijo de instalación predeterminado
AC_PREFIX_DEFAULT(run-in-place)

# Notificar verificando si RIP o no y ajustando prefix
#  - sin prefijo -> RIP, y explicitamente prefix=${BUILD_TOPLEVEL}
#  - con prefijo -> prefix
AC_MSG_CHECKING(prefijo de instalación)
if test "x$prefix" = "xNONE"; then
  RUN_IN_PLACE=yes
  AC_MSG_RESULT(RIP - ejecucion en el lugar)
  prefix=${BUILD_TOPLEVEL}
else
  RUN_IN_PLACE=no
  AC_MSG_RESULT($prefix)
fi
AC_SUBST(AUTODIRS)

# internationalisation package name:
PACKAGE="linuxcnc"
AC_SUBST([PACKAGE])

if test -f ../VERSION; then
    EMC2VERSION="$(cat ../VERSION)"
else
    EMC2VERSION="Undetermined"
fi
AC_SUBST([EMC2VERSION])

# Variables para los paths a grep y egrep
AC_PATH_PROG(GREP, grep)
AC_PATH_PROG(EGREP, egrep)

#
# check para pkg-config
#
PKG_PROG_PKG_CONFIG
if test -z "$PKG_CONFIG"; then
  AC_MSG_ERROR([se requiere pkg-config para construir LinuxCNC])
fi

#
# Sun transport-independent RPC (remote procedure call) library
# check para header de sistema rpc/rpc.h
#
AC_CHECK_HEADER([rpc/rpc.h],[], [
  if ! test -z "$PKG_CONFIG libtirpc"; then

    LIBTIRPC_CFLAGS=`pkg-config libtirpc --cflags`
    AC_SUBST([LIBTIRPC_CFLAGS])

    LIBTIRPC_LIBS=`pkg-config libtirpc --libs`
    AC_SUBST([LIBTIRPC_LIBS])

    CFLAGS="${CFLAGS:+$CFLAGS }$(pkg-config libtirpc --cflags)"
  fi

  # Update path
  AC_CHECK_LIB([tirpc], [get_myaddress], [], [
    AC_MSG_ERROR([No se encuentra tirpc])
  ])

  # Clear the cached check
  $as_unset ac_cv_header_rpc_rpc_h
  AC_CHECK_HEADER([rpc/rpc.h],[],[
    AC_MSG_ERROR([glibc rpc.h desaparecido, por favor instale libtirpc])
  ])
])


##############################################################################
# Sección 2                                                                  #
# Comprobación de tiempo real (RTAI o RT-Linux)                              #
# Subsección 2.1: argumentos para ./configure (--with-realtime)              #
# Subsección 2.2: si no se especificó ninguno antes, se debe buscar RTDIR    #
# Subsección 2.3: se encuentra RTS(RT-script), se consultan sus valores      #
##############################################################################

# preparar algunos valores por defecto
RTDIR=""
RTAI=""
RTPREFIX=""
KERNELDIR=""
RTFLAGS=""

NONDISTRIBUTABLE=false
AC_ARG_ENABLE(non-distributable,
    AS_HELP_STRING(
        [--enable-non-distributable],
        [Construir LinuxCNC incluso si un problema de licencia hiciera que los binarios resultantes no fueran distribuibles según los términos de la licencia de LinuxCNC]),
    [
        case "$enableval" in
        (yes) NONDISTRIBUTABLE=true ;;
        esac
    ])

##############################################################################
# Subsección 2.1                                                             #
# Verificar argumentos para ./configure                                      #
# aceptar --with-realtime=<path> desde la línea de comandos                  #
##############################################################################

RTDIR=""
AC_ARG_ENABLE(simulator,
    AS_HELP_STRING(
        [--enable-simulator],
        [Obsoleto! Utilice --with-realtime=uspace en su lugar.]
    ),
    [
        case "$enableval" in
        Y*|y*)
            RTS=uspace
            AC_MSG_WARN([--enable-simulator es obsoleto.  Utilice --with-realtime=uspace en su lugar.])
        ;;
        esac
    ])

AC_ARG_WITH(realtime,
    AS_HELP_STRING(
        [--with-realtime=PATH],
        [Path donde RTAI esta instalado, o "uspace" para userspace POSIX.]
    ),
    [   case "$withval" in
	"" | y | ye | yes | n | no)
	    AC_MSG_ERROR([Debe proporcionar un path o "uspace" para --with-realtime.]) ;;
	*)
            RTS="$withval" ;;
        esac
        ])
AC_SUBST([RTS])

TOOL_NML=no
AC_ARG_ENABLE(toolnml,
    AS_HELP_STRING(
        [--enable-toolnml],
        [Usar nml para tooldata. Por defecto, mmap]
    ),
    [
        case "$enableval" in
        N*|n*)
            TOOL_NML=no ;;
        Y*|y*)
            TOOL_NML=yes ;;
        esac
    ])
##############################################################################
# Subsección 2.2                                                             #
# 1. Si el usuario ha especificado un RT, es necesario comprobar su validez  #
#. Esto se hace comprobando las ubicaciones predeterminadas de un            #
# determinado script                                                         #
# (similar a 2)                                                              #
#                                                                            #
# 2. Si el usuario no ha especificado ningún RT explícitamente, se debe      #
# encontrar uno mediante una búsqueda. Para ello, el siguiente código        #
# buscará un script específico de RT:                                        #
# rtai-config para RTAI 3.x                                                  #
##############################################################################

## Notificar verificando rtai-config. En este punto, si RTS está vacío, debemos encontrar RT nosotros mismos.
AC_MSG_CHECKING([para rtai-config])
if test -z "$RTDIR"; then
  DIRS="/usr/realtime-`uname -r` /usr/realtime /usr/realtime* /usr /usr/src/rtai*"
else
  DIRS="$RTS"
fi
  RTAI_CONFIG=none
  for location in $DIRS; do
    for subdir in . scripts bin; do
    for signature in rtai; do
      candidate=$location/$subdir/$signature-config
      if test "$RTAI_CONFIG" = "none" -a -x $candidate; then
        RTAI_CONFIG=$candidate
      fi
    done
  done
done

AC_MSG_RESULT([$RTAI_CONFIG])
AC_SUBST([RTAI_CONFIG])

AC_PATH_PROG([XENOMAI_CONFIG], [xeno-config], [none])
AC_SUBST([XENOMAI_CONFIG])

# Notificar verificando
AC_MSG_CHECKING([para API(s) realtime a usar])
if test "$RTS" = uspace || test -z "$RTS" -a "$RTAI_CONFIG" = "none" ; then
  RTS=uspace
  if test "$RTAI_CONFIG" = "none" ; then
    USPACE_RTAI=
    CONFIG_USPACE_RTAI=n
    RTAI_LXRT_CFLAGS=
    RTAI_LXRT_LDFLAGS=
  else
    USPACE_RTAI=+lxrt
    CONFIG_USPACE_RTAI=y
    RTAI_LXRT_CFLAGS=`$RTAI_CONFIG --lxrt-cflags`
    RTAI_LXRT_LDFLAGS=`$RTAI_CONFIG --lxrt-ldflags`
    AC_DEFINE([USPACE_RTAI], [], [Definido si uspace realtime debe admitir opcionalmente RTAI LXRT])
  fi
  AC_SUBST([CONFIG_USPACE_RTAI])
  AC_SUBST([RTAI_LXRT_CFLAGS])
  AC_SUBST([RTAI_LXRT_LDFLAGS])

  if test "$XENOMAI_CONFIG" = "none" ; then
    USPACE_XENOMAI=
    CONFIG_USPACE_XENOMAI=n
    XENOMAI_CFLAGS=
    XENOMAI_LDFLAGS=
  else
    USPACE_XENOMAI=+xenomai
    CONFIG_USPACE_XENOMAI=y
    XENOMAI_CFLAGS=`$XENOMAI_CONFIG --skin posix --cflags`
    XENOMAI_LDFLAGS=`$XENOMAI_CONFIG --skin posix --ldflags`
    AC_DEFINE([USPACE_XENOMAI], [], [Definido si uspace realtime debe soportar opcionalmente Xenomai])
  fi
  AC_SUBST([CONFIG_USPACE_XENOMAI])
  AC_SUBST([XENOMAI_CFLAGS])
  AC_SUBST([XENOMAI_LDFLAGS])

  AC_MSG_RESULT([$RTS$USPACE_RTAI$USPACE_XENOMAI])

# Notificar verificando acceso PCI al espacio de usuario
  AC_MSG_CHECKING([habilitar el acceso PCI al espacio de usuario? ])
  AC_ARG_ENABLE(userspace-pci,
      AS_HELP_STRING(
        [--disable-userspace-pci],
        [Deshabilitar el acceso PCI al espacio de usuario (use esto si libudev no está disponible).],
      ),
      [
	case "$enableval" in
	(Y*|y*)
	    BUILD_UDEV=yes ;;
	(*)
	    BUILD_UDEV=no ;;
	esac
      ],
      [
        case "`uname -s`" in
        (*BSD*) BUILD_UDEV=no ;;
        (*) BUILD_UDEV=yes ;;
        esac
      ])
  AC_MSG_RESULT([$BUILD_UDEV])
  AC_SUBST([BUILD_UDEV])

  if test "$BUILD_UDEV" = "yes"; then
	# Notificar verificando libudev
    AC_MSG_CHECKING([para libudev])
    if pkg-config libudev >/dev/null 2>&1; then
        LIBUDEV_VER=`pkg-config udev --modversion`
        AC_MSG_RESULT(si - versión [$LIBUDEV_VER])
        LIBUDEV_CFLAGS=`pkg-config libudev --cflags`
        LIBUDEV_LIBS=`pkg-config libudev --libs`
    else
      AC_MSG_ERROR([libudev-dev no encontrada])
    fi
  else
    LIBUDEV_CFLAGS=-DNO_LIBUDEV
  fi
  AC_SUBST([LIBUDEV_CFLAGS])
  AC_SUBST([LIBUDEV_LIBS])
else
  RTS="$RTAI_CONFIG"
  AC_MSG_RESULT([$RTS])
fi


################################################################################
# Subsección 2.3                                                               #
# Ahora tenemos el script RT, es momento de consultarlo para obtener algunos   #
# valores. Puede proporcionarnos valores predeterminados como CC, CFLAGS, etc. #
# Vea cada caso para obtener más detalles                                      #
################################################################################

RTARCH=`uname -m | sed s/i.86/i386/`
case $RTS in
*/rtai-config)
    RTPREFIX=rtai
    RTAI_VERSION=$($RTS --version)
    RTAI=$(echo $RTAI_VERSION | cut -f 1 -d .)
    RTDIR=`$RTS --prefix`
    RTDIR=$(cd $RTDIR ; pwd -P )
    RTFLAGS=`$RTS --module-cflags`
    RTFLAGS="$RTFLAGS -DRTAI=$RTAI"
    KERNELDIR=`$RTS --linux-dir`
    RTARCH=`$RTS --arch`
    ULAPI_CFLAGS=`$RTS --lxrt-cflags`
    ULAPI_LDFLAGS=`$RTS --lxrt-ldflags`
;;
uspace|uspace+*)
    RTPREFIX=uspace
    RTDIR=$(pwd -P)
    ULAPI_CFLAGS=""
    RTFLAGS="-DUSPACE"
    BUILD_SYS=uspace
    MODEXT=.so
    MODULE_DIR=${prefix}/lib/linuxcnc/modules
    ULAPI_LDFLAGS=""
esac

if test "$RTS" != uspace; then
	# Notificar verificando
    AC_MSG_CHECKING([para localización de headers del kernel])
    AC_ARG_WITH(kernel-headers,
        AS_HELP_STRING(
            [--with-kernel-headers=PATH],
            [Localización de headers del kernel.]
        ),
	[
	    KERNELDIR="$withval"
	    AC_MSG_RESULT(usando valor especificado: $KERNELDIR)
	],[
	    AC_MSG_RESULT(usando valor desde RTS: $KERNELDIR)
	])
fi

AC_SUBST([RTAI])
AC_SUBST([RTPREFIX])
AC_SUBST([KERNELDIR])
AC_SUBST([RTFLAGS])
AC_SUBST([RTDIR])
AC_SUBST([RTARCH])
AC_SUBST([ULAPI_CFLAGS])
AC_SUBST([ULAPI_LDFLAGS])

case "$RTPREFIX" in
uspace)
    AC_DEFINE(RTAPI_USPACE, [], [Sistema Realtime es userspace]) ;;
rtai)
    AC_DEFINE(RTAPI_RTAI, [], [Sistema Realtime es RTAI]) ;;
esac


EMC2_HOME=`(cd .. ; pwd)`
AC_SUBST([EMC2_HOME])

AC_DEFINE_UNQUOTED([RTDIR], "$RTDIR", [Directorio del sistema realtime instalado])

################################################################################################
# Sección 3                                                                                    #
# Comprobaciones adicionales                                                                   #
# Subsección 3.1 - comprobar si hay un CC (debemos usar el mismo usado para RT)                #
# Subsección 3.2 - comprobar la versión del kernel, con parches RT                             #
# Subsección 3.3 - comprobar el directorio donde se pueden instalar los módulos                #
# Subsección 3.4 - comprobar GTK                                                               #
# Subsección 3.5 - comprobar los programas de utilidad necesarios para compilar y ejecutar EMC #
################################################################################################


#################################################################################
# Subsección 3.1 - comprobar si hay un CC (debemos usar el mismo usado para RT) #
#################################################################################
# Notificar verificando versión cc
AC_MSG_CHECKING([para versión cc])

if (test ${RTS##*/} = rtai-config ) ; then
  RTSCC=`$RTS --cc`
  AC_MSG_RESULT(encontrado $RTSCC en rtai-config)
else
  AC_MSG_RESULT(no especificado)
fi

AC_PROG_CC([$RTSCC gcc clang])
AC_PROG_CPP()

if test -z "$CC"; then
  AC_MSG_ERROR([no se encuentra cc acceptable])
elif ! test `$CC -dumpversion | cut -d '.' -f 1` -gt 2 ; then
  AC_MSG_ERROR([Compiladores mas antiguos que gcc 3.x ya no son soportados])
fi

# establecer flags para C y C++ si se soporta
for commonflag in -Wno-stringop-truncation ; do
  if echo "int main() { return 0;}" | $CC -Werror $commonflag -E - > /dev/null; then
    CFLAGS="${CFLAGS:+$CFLAGS }$flag"
    CXXFLAGS="${CXXFLAGS:+$CXXFLAGS }$flag"
  fi
done

# Notificar verificando linux/hidraw.h, interface raw a USB y Bluetooth HID 
AC_MSG_CHECKING([para usabilidad de linux/hidraw.h])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <sys/ioctl.h>
#include <linux/hidraw.h>
long hidiocgrawinfo = HIDIOCGRAWINFO;
    ]], [[]])],[
	HIDRAW_H_USABLE=yes
        AC_DEFINE([HIDRAW_H_USABLE], [], [Definido a 1 si linux/hidraw.h se puede utilizar y está definido HIDIOCGRAWINFO])
],[HIDRAW_H_USABLE=no])
AC_MSG_RESULT($HIDRAW_H_USABLE)
AC_SUBST([HIDRAW_H_USABLE])

#
# Check para rpc/rpc.h
#

# Notificar verificando rpc/rpc.h
AC_MSG_CHECKING([para usabilidad of rpc/rpc.h])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <rpc/rpc.h>
    ]], [[
struct sockaddr_in *foo;
get_myaddress(foo);
    ]])],[
        AC_MSG_RESULT(yes)
    ],[
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([No se puede utilizar rpc.h])
    
])

#
# check para libmodbus3
#

AC_ARG_WITH(
    [libmodbus],
    AS_HELP_STRING(
        [--with-libmodbus],
        [Especificar si desea o no compilar los controladores que utilizan libmodbus
        (predeterminado a "sí").]


    ),
    [WITH_LIBMODBUS=$withval],
    [WITH_LIBMODBUS=yes]
)

AS_IF(
    [test "x$WITH_LIBMODBUS" = "xyes"],
    [
		# Notificar verificando libmodbus3
        AC_MSG_CHECKING([para libmodbus3])
        if pkg-config libmodbus --atleast-version 3 >/dev/null 2>&1; then
            LIBMODBUS_VER=`pkg-config libmodbus --modversion`
            AC_MSG_RESULT(yes - version [$LIBMODBUS_VER])
            LIBMODBUS_CFLAGS=`pkg-config libmodbus --cflags`
            AC_SUBST([LIBMODBUS_CFLAGS])
            LIBMODBUS_LIBS=`pkg-config libmodbus --libs`
            AC_SUBST([LIBMODBUS_LIBS])
            AC_DEFINE(
                [HAVE_LIBMODBUS3],
                [yes],
                [definido si los encabezados y la biblioteca libmodbus3 están disponibles.]
            )
            AC_SUBST(HAVE_LIBMODBUS3, yes)
        else
          AC_MSG_RESULT(no)
          AC_MSG_ERROR([¡No se encontró libmodbus3!
Instálelo con "sudo apt-get install libmodbus-dev" o deshabilítelo con
"configure --without-libmodbus"])
        fi
    ]
)


#
# check para libusb-1.0
#

AC_ARG_WITH(
    [libusb-1.0],
    AS_HELP_STRING(
        [--with-libusb-1.0],
        [Especificar si desea o no compilar los controladores que utilizan libusb-1.0
        (predeterminado a "sí").]
    ),
    [WITH_LIBUSB10=$withval],
    [WITH_LIBUSB10=yes]
)

AS_IF(
    [test "x$WITH_LIBUSB10" = "xyes"],
    [
		# Notificar verificando libusb-1.0
        AC_MSG_CHECKING([for libusb-1.0])
        if pkg-config libusb-1.0 >/dev/null 2>&1; then
            AC_MSG_RESULT(yes)
            LIBUSB10_CFLAGS=`pkg-config libusb-1.0 --cflags`
            AC_SUBST([LIBUSB10_CFLAGS])
            LIBUSB10_LIBS=`pkg-config libusb-1.0 --libs`
            AC_SUBST([LIBUSB10_LIBS])
            AC_DEFINE(
                [HAVE_LIBUSB10],
                [yes],
                [Definido si los encabezados y la biblioteca libusb-1.0 están disponibles]
            )
            AC_SUBST(HAVE_LIBUSB10, yes)
        else
          AC_MSG_RESULT(no)
          AC_MSG_ERROR([¡No se encontró libusb-1.0!
Instálelo con "sudo apt-get install libusb-1.0-0-dev" o deshabilítelo con
"configure --without-libusb-1.0"])
        fi
    ]
)


#
# check para libgpiod  OJO, CORREGIR
#
PKG_CHECK_MODULES([LIBGPIOD], [libgpiod < 2.0.0],
            [AC_DEFINE_UNQUOTED([LIBGPIOD_VER], `pkg-config libgpiod --modversion | awk -F. '{print ($1 * 100)+($2) }'`,
            [versión libgpiod, no definida si no presente])
            LDFLAGS="$LDFLAGS $LIBGPIOD_LIBS"
            LIBGPIOD_VERSION=`pkg-config libgpiod --modversion`
            AC_MSG_NOTICE([encontrado libgpiod versión $LIBGPIOD_VERSION])
            AC_SUBST([LIBGPIOD_VER], `pkg-config libgpiod --modversion | awk -F. '{print ($1 * 100)+($2) }'`)],
            [AC_MSG_WARN([No se encuentra libgpiod, no se construye hal_gpio])])

#################################################################################
# Subsección 3.2 - comprobar la versión del kernel, con parches RT              #
#-------------------------------------------------------------------------------#
# Compruebe los encabezados del kernel para encontrar la versión de uts-kernel. #
# Esta debería ser una forma más confiable de determinar el kernel que          #
# se está usando para compilar: `uname -r` solo devuelve la                     #
# versión del kernel en ejecución, que puede no ser la misma contra la que      #
#se compila el código.                                                          #
#################################################################################

if test $RTS '!=' uspace; then
# Notificar verificando cadena de versión del kernel
AC_MSG_CHECKING([para la cadena de versión del kernel])

if test -e $KERNELDIR/include/linux/utsrelease.h; then
    VERSION_FILE=$KERNELDIR/include/linux/utsrelease.h
elif test -e $KERNELDIR/include/generated/utsrelease.h; then
    VERSION_FILE=$KERNELDIR/include/generated/utsrelease.h
else
    VERSION_FILE=$KERNELDIR/include/linux/version.h
fi
KERNEL_VERS=`$CC -E -dM ${VERSION_FILE} | grep UTS | cut -s -d '"' -f 2`

if test -z "$KERNEL_VERS"; then
    AC_MSG_ERROR(Cadena de versión del kernel no encontrada)
fi

AC_MSG_RESULT([$KERNEL_VERS])
AC_SUBST([KERNEL_VERS])
AC_DEFINE_UNQUOTED([RTAPI_KERNEL_VERSION], "$KERNEL_VERS", [Required kernel version])

# # Notificar verificando extension para modulos.
AC_MSG_CHECKING([para extension de modulos del kernel])
MODEXT=`$RTS --module-ext 2>/dev/null` || MODEXT=".o"
if test "x" = "x$MODEXT"; then
    MODEXT=".o"
fi
AC_MSG_RESULT([$MODEXT])

# # Notificar verificando si kbuild se usa
AC_MSG_CHECKING([Sistema de construcción del kernel])
# FIXME FIXME FIXME
# En este momento, se asumirá en función de la extensión de módulos del kernel.
EXT_RTFLAGS=""
BUILD_SYS="normal"

if test "x$MODEXT" = "x.ko" ; then
    BUILD_SYS="kbuild"
else
    EXT_RTFLAGS="-DEXPORT_SYMTAB"
fi

AC_ARG_WITH(kbuild,
    AS_HELP_STRING(
        [--with-kbuild],
        [Especificar si su kernel usa kbuild o no.]
    ),
    [
        case "$withval" in
        "" | y | ye | yes )
	    BUILD_SYS="kbuild"
          ;;
        n | no )
	    BUILD_SYS="normal"
          ;;
        esac
    ])
AC_MSG_RESULT([$BUILD_SYS])
fi
AC_SUBST([MODEXT])
AC_DEFINE_UNQUOTED([MODULE_EXT], "$MODEXT", [Extension para modulos realtime])
AC_SUBST([BUILD_SYS])
AC_SUBST([EXT_RTFLAGS])

###########################################################################################
# Subsección 3.3 - comprobar el moduledir donde se pueden instalar los módulos            #
#                                                                                         #
# Proporciona al usuario la posibilidad de especificarlo (--with-module-dir=<path>)       #
# Si no se especifica, por defecto dentro de /lib/modules/${KERNEL_VERS}/                 #
# en la ubicación donde se encuentra un módulo RT (mbuff.o o rtai_shed.o)                 #
# (¿quizás el valor predeterminado debería ser /lib/modules/$KERNEL_VERS/extra si existe? #
###########################################################################################

# # Notificar verificando directorio de instalación de módulos, similar a rtai's...
AC_MSG_CHECKING([para directorio de instalación de módulos])

AC_ARG_WITH(module-dir,
    AS_HELP_STRING(
        [--with-module-dir=PATH],
        [directorio de instalación para módulos del kernel.]
    ),
    [
        case "$withval" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([Debe suministrar un path para --with-module-dir.])
          ;;
        esac
        MODULE_DIR="$withval"
    ])


# si no se especifica, pregunta al script de configuración en tiempo real dónde está el directorio
# si el script de configuración devuelve un error, MODULE_DIR no establece en nada
if test "x" = "x$MODULE_DIR" ; then
  MODULE_DIR=`$RTS --module-dir 2>/dev/null`
  if ! test -d $MODULE_DIR ; then
    AC_MSG_ERROR([$RTS --module-dir no especificó un directorio existente. Debe especificarlo mediante --with-module-dir=...])
  fi
fi

if test "$RUN_IN_PLACE" = "yes"; then
    RIP_MODULE_DIR="${BUILD_TOPLEVEL}/rtlib"
    AC_MSG_RESULT([configurando para run-in-place])
    prefix=${BUILD_TOPLEVEL}
    if test "$TOOL_NML" = "yes"; then
       DEFAULT_NMLFILE=${prefix}/configs/common/linuxcnc_big.nml
    else
       DEFAULT_NMLFILE=${prefix}/configs/common/linuxcnc.nml
    fi
    DEFAULT_TOOLTABLE=${prefix}/configs/sim/sim.tbl
    AC_MSG_RESULT([$RIP_MODULE_DIR])
else
    RUN_IN_PLACE=no
    EMC2_HOME=$prefix
    if test "$TOOL_NML" = "yes"; then
       DEFAULT_NMLFILE=${prefix}/share/linuxcnc/linuxcnc_big.nml
    else
       DEFAULT_NMLFILE=${prefix}/share/linuxcnc/linuxcnc.nml
    fi
    DEFAULT_TOOLTABLE=${prefix}/share/doc/linuxcnc/examples/sample-configs/common/tool.tbl
    AC_MSG_RESULT([$MODULE_DIR])
fi

AC_SUBST([MODULE_DIR])
AC_SUBST([RIP_MODULE_DIR])
AC_SUBST([RUN_IN_PLACE])
AC_SUBST([DEFAULT_NMLFILE])
AC_SUBST([TOOL_NML])
AC_DEFINE_UNQUOTED([EMC2_DEFAULT_NMLFILE], "$DEFAULT_NMLFILE", [archivo nml por defecto])
AC_DEFINE_UNQUOTED([EMC2_DEFAULT_TOOLTABLE], "$DEFAULT_TOOLTABLE", [archivo nml por defecto])

########################################################################################
# Subsección 3.4: define los directorios predeterminados, utilizados en muchos lugares #
# Decidimos dónde van las cosas en función de RIP o instalación                        #
########################################################################################

fe () {
    # Funcion: Expandir completamente el argumento. Ejemplo:
    # ${datadir} -> ${datarootdir} -> ${prefix}/share -> /usr/local
    case "$1" in
    *\$*) eval fe \""$1"\" ;;
    *) echo "$1" ;;
    esac
}

if test "xyes" = "x$RUN_IN_PLACE"; then
    EMC2_LATENCY_SCRIPT=$EMC2_HOME/scripts/latency-test
    EMC2_LATENCY_HISTOGRAM_SCRIPT=$EMC2_HOME/scripts/latency-histogram
    EMC2_SCRIPT=$EMC2_HOME/scripts/linuxcnc
    EMC2_SUFFIX=" (run-in-place)"
    EMC2_ICON=$EMC2_HOME/linuxcncicon.png
    EMC2_BIN_DIR=$EMC2_HOME/bin
    EMC2_TCL_DIR=$EMC2_HOME/tcl
    EMC2_TCL_LIB_DIR=$EMC2_HOME/tcl
    EMC2_LANG_DIR=$EMC2_HOME/src/objects
    EMC2_PO_DIR=$EMC2_HOME/share/locale
    EMC2_HELP_DIR=$EMC2_HOME/docs/help
    EMC2_RTLIB_DIR=$EMC2_HOME/rtlib
    LINUXCNC_CONFIG_PATH="~/linuxcnc/configs:$EMC2_HOME/configs"
    EMC2_NCFILES_DIR=$EMC2_HOME/nc_files
    REALTIME=$EMC2_HOME/scripts/realtime
    EMC2_IMAGE_DIR=$EMC2_HOME
    HALLIB_DIR=$EMC2_HOME/lib/hallib
else
    EMC2_HOME=${prefix}
    EMC2_BIN_DIR=${prefix}/bin
    EMC2_LATENCY_SCRIPT=$EMC2_BIN_DIR/latency-test
    EMC2_LATENCY_HISTOGRAM_SCRIPT=$EMC2_BIN_DIR/latency-histogram
    EMC2_SCRIPT=$EMC2_BIN_DIR/linuxcnc
    EMC2_SUFFIX=""
    EMC2_ICON=linuxcncicon
    EMC2_TCL_DIR=${prefix}/lib/tcltk/linuxcnc
    EMC2_TCL_LIB_DIR=${prefix}/lib/tcltk/linuxcnc
    EMC2_LANG_DIR=${prefix}/lib/tcltk/linuxcnc/msgs
    EMC2_PO_DIR=${prefix}/share/locale
    EMC2_HELP_DIR=${prefix}/share/doc/linuxcnc
    case $MODULE_DIR in
        */linuxcnc*) EMC2_RTLIB_DIR=$MODULE_DIR ;;
	*) EMC2_RTLIB_DIR=$MODULE_DIR/linuxcnc
    esac
    LINUXCNC_CONFIG_PATH="~/linuxcnc/configs:/usr/local/etc/linuxcnc/configs:"$(eval echo $EMC2_HELP_DIR)"/examples/sample-configs"
    EMC2_NCFILES_DIR=${prefix}/share/linuxcnc/ncfiles
    REALTIME=${prefix}/lib/linuxcnc/realtime
    EMC2_IMAGE_DIR=$(fe "$datadir")/linuxcnc
    HALLIB_DIR=${prefix}/share/linuxcnc/hallib
fi

AC_DEFINE_UNQUOTED([EMC2_HOME], "$EMC2_HOME", [Directorio de prefijo o RIP])
AC_DEFINE_UNQUOTED([EMC2_BIN_DIR], "$EMC2_BIN_DIR", [Directorio para binarios])
AC_DEFINE_UNQUOTED([EMC2_TCL_DIR], "$EMC2_TCL_DIR", [Directorio para scripts tcl])
AC_DEFINE_UNQUOTED([EMC2_HELP_DIR], "$EMC2_HELP_DIR", [Directorio para archivos de ayuda])
AC_DEFINE_UNQUOTED([EMC2_RTLIB_DIR], "$EMC2_RTLIB_DIR", [Directorio de archivos de sistema tiempo real])
AC_DEFINE_UNQUOTED([EMC2_LANG_DIR], "$EMC2_LANG_DIR", [Directorio para archivos de traducción tcl])
AC_DEFINE_UNQUOTED([EMC2_PO_DIR], "$EMC2_PO_DIR", [Directorio para archivos de traducción po/mo])
AC_DEFINE_UNQUOTED([EMC2_NCFILES_DIR], "$EMC2_NCFILES_DIR", [Directorio para archivos nc])
AC_DEFINE_UNQUOTED([EMC2_IMAGE_DIR], "$EMC2_IMAGE_DIR", [Directorio para imágenes])

AC_SUBST([EMC2_BIN_DIR])
AC_SUBST([EMC2_TCL_DIR])
AC_SUBST([EMC2_TCL_LIB_DIR])
AC_SUBST([EMC2_HELP_DIR])
AC_SUBST([EMC2_RTLIB_DIR])
AC_SUBST([EMC2_LANG_DIR])
AC_SUBST([LINUXCNC_CONFIG_PATH])
AC_SUBST([EMC2_NCFILES_DIR])
AC_SUBST([REALTIME])
AC_SUBST([EMC2_IMAGE_DIR])
AC_SUBST([EMC2_SCRIPT])
AC_SUBST([EMC2_LATENCY_SCRIPT])
AC_SUBST([EMC2_LATENCY_HISTOGRAM_SCRIPT])
AC_SUBST([EMC2_SUFFIX])
AC_SUBST([EMC2_ICON])
AC_SUBST([HALLIB_DIR])

# # coordinar aplicaciones auxiliares instaladas (no dependientes de RIP o deb):
LINUXCNC_AUX_GLADEVCP=/usr/share/linuxcnc/aux_gladevcp
LINUXCNC_AUX_EXAMPLES=/usr/share/linuxcnc/aux_examples
AC_SUBST([LINUXCNC_AUX_GLADEVCP])
AC_SUBST([LINUXCNC_AUX_EXAMPLES])

##############################################################################
# Subsecciónn 3.5 - check para GTK                                           #
# FIXME: permitir habilitar o deshabilitar la línea de comandos              #
##############################################################################

# Notificar verificando glib, libreria GTK
AC_MSG_CHECKING([para glib])
if pkg-config glib-2.0 >/dev/null 2>&1; then
  GLIB_VER=`pkg-config glib-2.0 --modversion`
  AC_MSG_RESULT(yes - $GLIB_VER)
  GLIB_CFLAGS=`pkg-config glib-2.0 --cflags`
  GLIB_LIBS=`pkg-config glib-2.0 --libs`
else
  AC_MSG_ERROR(no se requieren --hasta que glib sea opcional)
fi


AC_ARG_ENABLE(gtk,
    AS_HELP_STRING(
        [--disable-gtk],
        [Deshabilita las partes de LinuxCNC que dependen de GTK.]
    ),
    [
	case "$enableval" in
	Y*|y*)
	    BUILD_GTK=yes ;;
	*)
	    BUILD_GTK=no ;;
	esac
    ],
    [BUILD_GTK=yes])

if test "$BUILD_GTK" = "yes"; then
	# Notificar verificando GTK 3
  AC_MSG_CHECKING([para GTK 3.22.4 o superior])
  if pkg-config gtk+-3.0 --atleast-version 3.22.4 >/dev/null 2>&1; then
    GTK_VER=`pkg-config gtk+-3.0 --modversion`
    AC_MSG_RESULT(yes - $GTK_VER)
    GTK_CFLAGS=`pkg-config gtk+-3.0 --cflags`
    GTK_LIBS=`pkg-config gtk+-3.0 --libs`
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([Falta GTK3. Instálelo con "sudo apt install libgtk-3-dev" o especifique --disable-gtk para omitir las partes de LinuxCNC que dependen de GTK])
  fi
fi

AC_SUBST(HAVE_GNOMEPRINT)
AC_SUBST(GTK_VER)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

AC_ARG_ENABLE(gtk2,
    AS_HELP_STRING(
        [--disable-gtk2],
        [Deshabilita las partes de LinuxCNC que dependen de GTK2. Solo necesarias para classicladder)]
    ),
    [
	case "$enableval" in
	Y*|y*)
	    BUILD_GTK2=yes ;;
	*)
	    BUILD_GTK2=no ;;
	esac
    ],
    [BUILD_GTK2=yes])

if test "$BUILD_GTK2" = "yes"; then
	# Notificar verificando GTK 2
  AC_MSG_CHECKING([para GTK 2.4.0 o superior])
  if pkg-config gtk+-2.0 --atleast-version 2.4.0 >/dev/null 2>&1; then
    GTK2_VER=`pkg-config gtk+-2.0 --modversion`
    AC_MSG_RESULT(yes - $GTK2_VER)
    GTK2_CFLAGS=`pkg-config gtk+-2.0 --cflags`
    GTK2_LIBS=`pkg-config gtk+-2.0 --libs`
	# Notificar verificando libgnomeprintui
    AC_MSG_CHECKING(for libgnomeprintui-2.2)
    if pkg-config --exists libgnomeprintui-2.2; then
      AC_MSG_RESULT(yes)
      GTK2_CFLAGS="$GTK2_CFLAGS `pkg-config --cflags libgnomeprintui-2.2`"
      GTK2_LIBS="$GTK2_LIBS `pkg-config --libs libgnomeprintui-2.2`"
      HAVE_GNOMEPRINT=yes
    else
      AC_MSG_RESULT(no -- no será posible imprimir desde classicladder.)
      HAVE_GNOMEPRINT=
    fi
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([Falta GTK2. Instálelo con "sudo apt install libgtk2.0-dev" o especifique --disable-gtk2 para omitir la compilación de classicladder])
  fi
fi

AC_SUBST(GTK2_VER)
AC_SUBST(GTK2_LIBS)
AC_SUBST(GTK2_CFLAGS)

############################################################################################
# Subsección 3.6: comprobar los programas de utilidad necesarios para crear y ejecutar EMC #
#                                                                                          #
# Comprobar todos los programas de utilidad que necesitamos para crear e instalar EMC      #
# así como aquellos que usamos en los scripts de EMC...                                    #
############################################################################################

SPATH=$PATH:/usr/local/sbin:/usr/sbin:/sbin
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PATH_PROG(AR,ar)
AC_PATH_PROG(INSTALL, install)

AC_PATH_PROG(SED, sed, "none")
if test $SED = "none"
then
    AC_MSG_ERROR([sed no encontrado])
fi

AC_PATH_PROG(PS, ps, "none")
if test $PS = "none"
then
    AC_MSG_ERROR([ps no encontrado])
fi

AC_PATH_PROG(KILL, kill, "none")
if test $KILL = "none"
then
    AC_MSG_ERROR([kill no encontrado])
fi

AC_PATH_PROG(WHOAMI, whoami, "none")
if test $WHOAMI = "none"
then
    AC_MSG_ERROR([whoami no encontrado])
fi

AC_PATH_PROG(AWK, awk, "none")
if test $AWK = "none"
then
    AC_MSG_ERROR([awk no encontrado])
fi

if test "$RTS" != uspace; then
    # Utilidades de módulo no necesarias para uspace
    AC_PATH_PROG(INSMOD, insmod, "none", $SPATH)
    if test $INSMOD = "none"
    then
        AC_MSG_ERROR([insmod no encontrado])
    fi

    AC_PATH_PROG(RMMOD, rmmod, "none", $SPATH)
    if test $RMMOD = "none"
    then
        AC_MSG_ERROR([rmmod no encontrado])
    fi

    AC_PATH_PROG(LSMOD, lsmod, "none", $SPATH)
    if test $LSMOD = "none"
    then
        AC_MSG_ERROR([lsmod no encontrado])
    fi
fi

AC_PATH_PROG(PIDOF, pidof, "none", $SPATH)
if test $PIDOF = "none"
then
    AC_MSG_ERROR([pidof no encontrado])
fi

AC_PATH_PROG(IPCS, ipcs, "none")
if test $IPCS = "none"
then
    AC_MSG_ERROR([ipcs no encontrado])
fi

AC_PATH_PROG(FUSER, fuser, "none", $SPATH)
if test $FUSER = "none"
then
    AC_MSG_ERROR([fuser no encontrado])
fi

AC_PATH_PROGS(YAPPS, yapps yapps2, "none", $SPATH)
if test $YAPPS = "none"
then
    AC_MSG_ERROR([yapps/yapps2 no encontrado])
fi
AC_SUBST(YAPPS)

# MANDB vacío se maneja en doc/Submakefile
AC_PATH_PROG(MANDB, mandb, "")

AC_PATH_PROG(INTLTOOL_EXTRACT, intltool-extract, "none", $SPATH)
if test $INTLTOOL_EXTRACT = "none"
then
    AC_MSG_ERROR([intltool-extract no encontrado])
fi

AC_ARG_WITH(rmmod,
    AS_HELP_STRING(
        [--with-rmmod=PATH],
        [Ubicación del programa rmmod a utilizar.]
    ),
    [
	case "$withval" in
	"" | y | ye | yes)
	    AC_MSG_ERROR([Debe proporcionar un argumento a --with-rmmod.])
	  ;;
	n | no | none)
	    RMMOD=""
	  ;;
	*)
	    RMMOD="$withval"
	esac
    ],
    [
            RMMOD="$EMC2_BIN_DIR/linuxcnc_module_helper remove"
    ])

AC_ARG_WITH(insmod,
    AS_HELP_STRING(
        [--with-insmod=PATH],
        [Ubicación del programa insmod a utilizar.]
    ),
    [
	case "$withval" in
	"" | y | ye | yes)
	    AC_MSG_ERROR([Debe proporcionar un argumento a --with-insmod.])
	  ;;
	n | no | none)
	    INSMOD=""
	  ;;
	*)
	    INSMOD="$withval"
	esac
    ],
    [
            INSMOD="$EMC2_BIN_DIR/linuxcnc_module_helper insert"
    ])

AC_PATH_PROGS(YAPPS, [yapps yapps2])
if test -z "$YAPPS"; then
  AC_MSG_ERROR([Se requiere yapps para construir LinuxCNC
instalar con "sudo apt-get install yapps2"])
fi
AC_SUBST(YAPPS)

##################################################################################
# Subsección 3.7: comprobar los programas necesarios para crear la documentación #
#                                                                                #
# Comprobar los programas que necesitamos para crear e instalar la documentación.#
# (Opcional, si no están presentes, simplemente no cree la documentación).       #
##################################################################################

AX_BOOST_BASE(,,AC_MSG_ERROR([libboost no encontrado]))
AX_PYTHON
AC_PATH_PROG(PYTHON,[$PYTHON_BIN])
AX_PYTHON_DEVEL(,,AC_MSG_ERROR([python-dev no encontrado]))
AX_BOOST_PYTHON(,,AC_MSG_ERROR([libboost-python no encontrado]))

# Notificar verificando documentación
AC_MSG_CHECKING([si se construye la documentación])
AC_ARG_ENABLE(build-documentation,
    AS_HELP_STRING(
        [--enable-build-documentation[=FORMAT]],
        [ Compilación de la documentación. FORMAT puede ser "Y" para ambos formatos o "pdf" o "html" si solo se desea un formato.]
    ),
    [
        case "$enableval" in
        Y*|y*|html,pdf|pdf,html)
            BUILD_DOCS=yes
            BUILD_DOCS_PDF=yes
            BUILD_DOCS_HTML=yes
            AC_MSG_RESULT([solicitado PDF y HTML])
        ;;
        html)
            BUILD_DOCS=yes
            BUILD_DOCS_PDF=no
            BUILD_DOCS_HTML=yes
            AC_MSG_RESULT([solicitado HTML])
	;;
        pdf)
            BUILD_DOCS=yes
            BUILD_DOCS_PDF=yes
            BUILD_DOCS_HTML=no
            AC_MSG_RESULT([solicitado PDF])
	;;
        *)
            BUILD_DOCS=no
            AC_MSG_RESULT([no])
        ;;
    esac
    ],
    [
        BUILD_DOCS=no
        AC_MSG_RESULT([no])
    ])

# Programas necesarios para construir toda la documentación
if ( test "$BUILD_DOCS" = "yes" ) ; then
    AC_PATH_PROG(ASCIIDOC,asciidoc,"none")
    if ( test "none" = "$ASCIIDOC" ) ; then
	AC_MSG_WARN([sin AsciiDoc, no se puede construir la documentación])
        BUILD_DOCS=no
    fi

    AC_PATH_PROG(A2X,a2x,"none")
    if ( test "none" = "$A2X" ) ; then
	AC_MSG_WARN([sin a2x, no se puede construir la documentación])
        BUILD_DOCS=no
    fi

	# Notificar verificando
    AC_MSG_CHECKING([whether to specify latex.encoding])
    temp_asciidoc=`mktemp --suffix=.txt`
    cat > $temp_asciidoc <<EOF
:lang: fr
:ascii-ids:

....
Matúš hat Geläut am Fuß
....
EOF
    if $A2X --dblatex-opts "-P latex.encoding=utf8" $temp_asciidoc > /dev/null 2>&1;
    then
        A2X_LATEX_ENCODING="-P latex.encoding=utf8"
        AC_MSG_RESULT(yes)
    else
        A2X_LATEX_ENCODING=""
        AC_MSG_RESULT(no)
    fi
    AC_SUBST(A2X_LATEX_ENCODING)
    rm -f $temp_asciidoc ${temp_asciidoc%.txt}.pdf

    AC_PATH_PROG(DBLATEX,dblatex,"none")
    if ( test "none" = "$DBLATEX" ) ; then
	AC_MSG_WARN([sin dblatex, no se puede construir la documentación])
        BUILD_DOCS=no
    fi

    if ( test "$BUILD_DOCS" = "yes" ) ; then
		# Notificar verificando versión de dblatex
        AC_MSG_CHECKING([versión de dblatex])
        set -- `dblatex --version`; DBLATEX_VER=$3
        set -- `echo $DBLATEX_VER | sed 's/[[.-]]/ /g'`
        micro=`echo $3 | sed 's/\([[0-9]]*\).*/\1/g'`

        if test $1 -eq 0 -a \( $2 -lt 2 -o \( $2 -eq 2 -a ${micro:-0} -lt 12 \) \); then
            AC_MSG_WARN([versión de dblatex $DBLATEX_VER menor que 0.2.12.
No se puede construir la documentación.])
            BUILD_DOCS=no
        else
            AC_MSG_RESULT([$DBLATEX_VER])
        fi
    fi

    if ( test "$BUILD_DOCS" = "yes" ) ; then
        AC_PATH_PROG(SOURCE_HIGHLIGHT,source-highlight,"none")
        if ( test "none" = "$SOURCE_HIGHLIGHT" ) ; then
            AC_MSG_WARN([sin source-highlight, no se puede construir la documentación])
            BUILD_DOCS=no
        fi
    fi

    if ( test "$BUILD_DOCS" = "yes" ) ; then
        AC_PATH_PROG(CONVERT,convert,"none")
        if ( test "none" = "$CONVERT" ) ; then
            AC_MSG_WARN([sin convert, no se puede construir la documentación])
            BUILD_DOCS=no
        fi
    fi

    if ( test "$BUILD_DOCS" = "yes" ) ; then
        AC_PATH_PROG(GS,gs,"none")
        if ( test "none" = "$GS" ) ; then
            AC_MSG_WARN([sin gs, no se puede construir la documentación])
            BUILD_DOCS=no
        fi
    fi
fi

# Programs required only for building the PDF documentation
if ( test "$BUILD_DOCS_PDF" = "yes" ) ; then
    AC_PATH_PROG(PDFLATEX,pdflatex,"none")
    if ( test "none" = "$PDFLATEX" ) ; then
	AC_MSG_WARN([sin pdflatex, no se puede construir la documentación en PDF])
        BUILD_DOCS=no
    fi
fi

# Programs required only for building the HTML documentation
if ( test "$BUILD_DOCS_HTML" = "yes" ) ; then
    AC_PATH_PROG(XSLTPROC,xsltproc,"none")
    if ( test "none" = "$XSLTPROC" ) ; then
	AC_MSG_WARN([sin xsltproc, no se puede construir la documentación en HTML])
        BUILD_DOCS=no
    fi

    AC_PATH_PROG(DVIPNG,dvipng,"none")
    if ( test "none" = "$DVIPNG" ) ; then
	AC_MSG_WARN([sin dvipng, no se puede construir la documentación en HTML])
        BUILD_DOCS=no
    fi

# Notificar verificando
    AC_MSG_CHECKING([for HTML support in groff])
    if ! groff -Thtml < /dev/null > /dev/null 2>&1 ; then
        AC_MSG_WARN([sin groff -Thtml, no se puede construir la documentación en HTML])
        BUILD_DOCS=no
    else
        AC_MSG_RESULT(yes)
    fi

    AC_PATH_PROG(CHECKLINK,checklink,"none")
    if ( test "none" = "$CHECKLINK" ) ; then
	AC_MSG_WARN([sin checklink, no se puede construir la documentación en HTML
instalar con "sudo apt-get install w3c-linkchecker"])
        BUILD_DOCS=no
    fi
fi

AC_ARG_ENABLE(build-documentation-translation,
    AS_HELP_STRING(
        [--disable-build-documentation-translation],
        [Deshabilitar la construcción de documentación en idiomas distintos del inglés.]
    ),
    [
    case "$enableval" in
	Y*|y*)
	    BUILD_DOCS_TRANSLATED_EXPLICIT=yes ;;
	no)
	    BUILD_DOCS_TRANSLATED_EXPLICIT=no ;;
	*)
	    BUILD_DOCS_TRANSLATED_EXPLICIT=$enableval ;;
	esac
    ],
    [
        BUILD_DOCS_TRANSLATED_EXPLICIT=not_set
    ])

if ( test "$BUILD_DOCS" = "yes" && test "$BUILD_DOCS_TRANSLATED_EXPLICIT" != "no") ; then
    AC_PATH_PROG(PO4A, po4a, "none")
    BUILD_DOCS_TRANSLATED=yes
    if test $PO4A = "none"
    then
        if test "$BUILD_DOCS_TRANSLATED_EXPLICIT" = "not_set"
        then
            AC_MSG_WARN([po4a no encontrado, no se construyen documentos traducidos])
        else
            AC_MSG_ERROR([po4a no encontrado, no se construyen documentos traducidos])
        fi
        BUILD_DOCS_TRANSLATED=no
    else
        # Version 0.35 support asciidoc without tables
        # Version 0.56 support the tablecells asciidoc option
        # Version 0.62 correctly write UTF-8 into translated adocs
        # Version 0.65 support grouping several doc files into different POTs
        # Version 0.66 handle empty table cells in asciidoc
        # Version 0.67 handle enforced linbreaks
        V=$(po4a --version| awk '/po4a version/ {print $3}')
        if dpkg --compare-versions 0.67 gt "$V"; then
            if test "$BUILD_DOCS_TRANSLATED_EXPLICIT" = "not_set"
            then
                AC_MSG_WARN([La versión de po4a es demasiado antigua, se necesita la versión 0.67 o más nueva, no se crean documentos traducidos])
            else
                AC_MSG_ERROR([La versión de po4a es demasiado antigua, se necesita la versión 0.67 o más nueva, no se crean documentos traducidos])
            fi
	    BUILD_DOCS_TRANSLATED=no
        fi
    fi
fi
AC_SUBST(BUILD_DOCS)
AC_SUBST(BUILD_DOCS_PDF)
AC_SUBST(BUILD_DOCS_HTML)
AC_SUBST(BUILD_DOCS_TRANSLATED)

if test "$RTS" '!=' uspace
then
for m in adeos rtai_hal rtai_ksched rtai_sched rtai_fifos rtai_sem rtai_math \
; do
	# Notificar verificando para la lista $m
    AC_MSG_CHECKING([para $m])
    TMP=$(find $MODULE_DIR -name $m$MODEXT 2>/dev/null)
    if test -z "$TMP"; then
        TMP=$(find /lib/modules/$KERNEL_VERS -name $m$MODEXT)
    fi
    eval MODPATH_$m=${TMP/$KERNEL_VERS/\\\$\\\(uname\\\ -r\\\)}
    AC_MSG_RESULT([${TMP:-no encontrado}])
done

AC_SUBST(MODPATH_adeos)
AC_SUBST(MODPATH_rtai_hal)
AC_SUBST(MODPATH_rtai_ksched)
AC_SUBST(MODPATH_rtai_sched)
AC_SUBST(MODPATH_rtai_fifos)
AC_SUBST(MODPATH_rtai_sem)
AC_SUBST(MODPATH_rtai_math)
fi

# Checks para headers.

##############################################################################
# Sección 4.1 – Encabezados importantes, funciones y definiciones globales.  #
#                                                                            #
##############################################################################

# Esta sección se inserta en config.h.in cuando se ejecuta autoheader.
# Las definiciones globales y comunes deberían estar aquí en lugar de duplicarse en
# varios lugares. Mantenga las construcciones raras fuera de esto y podrá
# usarse tanto en el espacio del kernel como en el espacio del usuario.
AH_TOP([/********************************************************************
* Description: config.h
*
*	Definiciones comunes utilizadas en muchos archivos fuente de emc2
*   Para agregar a este archivo, edite la Sección 4.1 en configure.ac
*   y luego ejecute autoheader.
*
* Author: Autogenerado por autoheader
* License: LGPL Version 2
* System: Linux
*
* Copyright (c) 2004 All rights reserved.
********************************************************************/
#ifndef EMC2_CONFIG_H
#define EMC2_CONFIG_H
])

# Chequeo io.h
AC_CHECK_HEADERS([sys/io.h])

# Incluir tal cual, con clave unica _GNU_SOURCE
AH_VERBATIM([_GNU_SOURCE],
[/* Habilitar extensiones GNU en sistemas que las tengan.*/
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
])

# Chequeo sys/wait.h
AC_HEADER_SYS_WAIT
# Chequeo de funcion semtimedop. Ver $ man SEMOP (System V semaphore operations)
AC_CHECK_FUNCS(semtimedop)

# Notificar verificando optreset
AC_MSG_CHECKING([para optreset])
# compilar/linkar 
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <unistd.h>
extern int optreset;
    ]], [[ optreset = 1; ]])],[
	HAVE_OPTRESET=yes
        AC_DEFINE([HAVE_OPTRESET], [], [Definir como 1 si getopt tiene la extensión BSD 'optreset'])
],[HAVE_OPTRESET=no])
AC_MSG_RESULT($HAVE_OPTRESET)
AC_SUBST([HAVE_OPTRESET])



LIBS_hold=$LIBS
LIBS=
AC_SEARCH_LIBS(dlopen, [c dl], [
    case "$LIBS" in
    (-lc) LIBDL= ;;
    (*)   LIBDL="$LIBS"
    esac

    AC_DEFINE([LIBDL], [$LIBDL], [Define la biblioteca de donde proviene dlopen, si no es parte de libc])
], [AC_MSG_ERROR([no encontrado])])
AC_SUBST([LIBDL])
LIBS=$LIBS_hold

AC_SEARCH_LIBS(clock_nanosleep, [rt], [
    AC_DEFINE([HAVE_CLOCK_NANOSLEEP], 1,
        [Definir en 1 si clock_nanosleep está disponible (en -lc o -lrt)])
])

AH_BOTTOM(#endif
)

##############################################################################
# Section 5 - Check for tcl/tk support                                       #
#                                                                            #
# Test for tcl/tk headers fails if they are not in /usr/include              #
# so testing is done by finding tclConfig.sh and tkConfig.sh                 #
# if the search fails the traditional check is done                          #
##############################################################################
# Notificar verificando tcl
AC_MSG_CHECKING([for tcl])

AC_ARG_WITH(tclConfig,
    AS_HELP_STRING(
        [--with-tclConfig=PATH],
        [Especifique el path a tclConfig.]
    ),
    [
        TCLCONFIG=$withval
    ],[
        TCLCONFIG=
    ])

# if not specified, search for tclConfig.sh
if (test "x$TCLCONFIG" = "x"); then
    TCLCONFIG=$(find /usr/lib* -maxdepth 2 -name tclConfig.sh | sort -r | head -n1)
    if (test "x$TCLCONFIG" = "x"); then
	TCLCONFIG=$(find /usr/local/lib* -maxdepth 2 -name tclConfig.sh | sort -r | head -n1)
    fi
fi

if (test "x$TCLCONFIG" = "x"); then
   AC_MSG_RESULT([tclConfig.sh no encontrado, intentando tcl.h y libs.
		  Si no funciona, pruebe con ./configure --with-tclConfig=<path to tclConfig.sh>])
   AC_CHECK_HEADERS([tcl.h])
   AC_SEARCH_LIBS(Tcl_Init,[tcl tcl8.7 tcl8.6],[],[AC_MSG_ERROR([tcl lib not encontrada])])
else
# tclConfig.sh found,
# extracting vars
   AC_MSG_RESULT([$TCLCONFIG encontrada])
   . "$TCLCONFIG"
   TCL_CFLAGS="$TCL_INCLUDE_SPEC -DUSE_TCL_STUBS"
   TCL_LIBS="$TCL_STUB_LIB_SPEC"
fi

# Notificar verificando
AC_MSG_CHECKING([para tk])

AC_ARG_WITH(tkConfig,
    AS_HELP_STRING(
        [--with-tkConfig=PATH],
        [Especifica el path a tkConfig.]
    ),
    [
        TKCONFIG=$withval
    ],[
        TKCONFIG=
    ])

# if not specified, search for tclConfig.sh
if test -z "$TKCONFIG"; then
    TKCONFIG=$(dirname $TCLCONFIG)/tkConfig.sh
    if ! test -f $TKCONFIG; then unset TKCONFIG; fi
fi
if (test "x$TKCONFIG" = "x"); then
    # search for tkConfig.sh
    TKCONFIG=$(find /usr/lib -maxdepth 2 -name tkConfig.sh | sort -r | head -n1)
    if (test "x$TKCONFIG" = "x"); then
	TKCONFIG=$(find /usr/local/lib -maxdepth 2 -name tkConfig.sh | head -n1)
    fi
fi

if (test "x$TKCONFIG" = "x"); then
   AC_MSG_RESULT([tkConfig.sh no encontrado, intentando con tk.h y libs.
		  Si no funciona, pruebe con  ./configure --with-tkConfig=<path to tkConfig.sh>])
   AC_CHECK_HEADERS([tk.h])
   AC_SEARCH_LIBS(Tk_Init,[tk tcl8.7 tcl8.6],[],[AC_MSG_ERROR([tk lib no encontrada])])
else
# tkConfig.sh found,
# extracting vars
   AC_MSG_RESULT([$TKCONFIG encontrada])
   . "$TKCONFIG"
   TK_CFLAGS="$TK_INCLUDE_SPEC $TK_XINCLUDES"
   TK_LIBS="$TK_LIBS $TK_LIB_SPEC"
fi

if ! test -z "$TCL_VERSION" && ! test -z "$TK_VERSION" && ! test "$TCL_VERSION" = "$TK_VERSION" ; then
    AC_MSG_ERROR([Tcl y Tk deben ser de la misma versión, pero configure encuentra 
Tcl $TCL_VERSION y Tk $TK_VERSION.  Puede usar --with-tkConfig= and --with-tclConfig=
para ignorar las versiones autodetectadas.])
fi

if test -f $TCL_EXEC_PREFIX/bin/wish$TCL_VERSION; then
    WISH=$TCL_EXEC_PREFIX/bin/wish$TCL_VERSION
elif test -f $TCL_EXEC_PREFIX/bin/wish; then
    WISH=$TCL_EXEC_PREFIX/bin/wish
else
    WISH=wish
fi

if test -f $TCL_EXEC_PREFIX/bin/tclsh$TCL_VERSION; then
    TCLSH=$TCL_EXEC_PREFIX/bin/tclsh$TCL_VERSION
elif test -f $TCL_EXEC_PREFIX/bin/tclsh; then
    TCLSH=$TCL_EXEC_PREFIX/bin/tclsh
else
    TCLSH=tclsh
fi

AC_ARG_ENABLE(check-runtime-deps,
    AS_HELP_STRING(
        [--disable-check-runtime-deps],
        [Omitir comprobaciones de requisitos de tiempo de ejecución que no son necesarios durante el
        proceso de compilación.]
    ),
    [RUNTIME_CHECK=$enableval],
    [RUNTIME_CHECK=yes])
AC_MSG_CHECKING([si chequear para dependencias runtime])
AC_MSG_RESULT($RUNTIME_CHECK)

if test "$RUNTIME_CHECK" = "yes"; then
	# Notificar verificando
    AC_MSG_CHECKING([para BWidget usando $TCLSH])
    if (unset DISPLAY; echo ["catch { package require bwidget }; exit [expr [lsearch [package names] BWidget] == -1]"] | $TCLSH); then
        AC_MSG_RESULT([encontrado])
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([BWidget no encontrado!
    instalar con "sudo apt-get install bwidget"])
    fi

	# Notificar verificando
    AC_MSG_CHECKING([para BLT usando $TCLSH])
    if (unset DISPLAY; echo ['if [catch {package require BLT}] { exit 1; } else { exit 0; }'] | $TCLSH); then
        AC_MSG_RESULT([encontrado])
    else
        AC_MSG_RESULT([no encontrado])
    fi

	# Notificar verificando
    AC_MSG_CHECKING([for tclX using $TCLSH])
    if (unset DISPLAY; echo ["catch { package require Tclx }; exit [expr [lsearch [package names] Tclx] == -1]"] | $TCLSH); then
        AC_MSG_RESULT([encontrado])
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([Tclx no encontrado!
    instalar con "sudo apt-get install tclx"])
    fi

	# Notificar verificando
    AC_MSG_CHECKING([para módulo python pango])
    if $PYTHON -c "import gi;gi.require_version('Pango', '1.0');from gi.repository import Pango"; then
        AC_MSG_RESULT([encontrado])
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR(
[Python pango module no encontrado!
instalar con "sudo apt-get install python3-gi"])
    fi
fi

AC_SUBST([TCL_DBGX])
AC_SUBST([TK_DBGX])
AC_SUBST([TCL_CFLAGS])
AC_SUBST([TK_CFLAGS])
AC_SUBST([TCL_LIBS])
AC_SUBST([TK_LIBS])
AC_SUBST([WISH])
AC_SUBST([TCLSH])

##############################################################################
# Section 6 - Miscellaneous support                                          #
#                                                                            #
# Subsection 6.1. - provides DATE to output it to the generated files        #
# Subsection 6.2. - user option to convince make to be quiet during make     #
##############################################################################

# provides DATE in order to output it to the generated files
DATE=$(date)
AC_SUBST([DATE])
AC_PATH_XTRA
#X_CFLAGS gets set
#X_LIBS and X_EXTRA_LIBS likewise

AC_SUBST([X_LIBS])
AC_SUBST([X_CFLAGS])

AC_CHECK_HEADERS(X11/extensions/Xinerama.h,
	    [HAVE_XINERAMA=yes],
	    [HAVE_XINERAMA=no])

AC_CHECK_LIB(Xinerama, XineramaQueryExtension,
	    [],
	    [HAVE_XINERAMA=no])

if test "x$HAVE_XINERAMA" = "xno"; then
    AC_MSG_ERROR([Libreria o headers Xinerama no encontrados])
fi

#clean out LIBS
temp_LIBS=$LIBS
LIBS=

# check for readline.h and friends, optional for halcmd
AC_CHECK_HEADERS(readline/readline.h, [HAVE_READLINE=yes], [HAVE_READLINE=no])
AC_CHECK_HEADERS(readline/history.h, [], [HAVE_READLINE=no])

echo "Intentando readline sin -ltermcap"
AC_CHECK_LIB(readline, readline, [HAVE_READLINE_NOTERMCAP=yes], [])
unset ac_cv_lib_readline_readline
echo "Intentando readline con -ltermcap"
AC_CHECK_LIB(readline, readline, [HAVE_READLINE_TERMCAP=yes], [], -ltermcap)
unset ac_cv_lib_readline_readline

if test "x$HAVE_READLINE_NOTERMCAP" = "xyes"; then
    echo "Readline no requiere -ltermcap"
    READLINE_LIBS="-lreadline"
elif test "x$HAVE_READLINE_TERMCAP" = "xyes"; then
    echo "Readline requiere -ltermcap"
    READLINE_LIBS="-lreadline -ltermcap"
else
    HAVE_READLINE=no
fi

# Notificar verificando
AC_MSG_CHECKING([si la licencia readline es compatible con GPL-2])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include <stdio.h>
        #include <readline/readline.h>
    ]], [[
        #if RL_VERSION_MAJOR > 5
        #error Readline versión 6 y superiores no son compatibles con GPL-2
        #endif
    ]])],[AC_MSG_RESULT(yes)],[
        AC_MSG_RESULT(no)
        AC_MSG_WARN(
[El binario LinuxCNC que estás creando puede no ser
distribuible debido a una incompatibilidad de licencia con LinuxCNC (algunas partes
solo GPL-2) y Readline versión 6 y posteriores (GPL-3 o posterior).])
        if $NONDISTRIBUTABLE; then
            AC_MSG_WARN(
[User requested to enable non-distributable builds.  Continuing.])
        else
            AC_MSG_ERROR(
[Para configurar LinuxCNC de esta manera, debe
invocar configure con "--enable-non-distributable=yes". Tenga en cuenta que en
sistemas basados ​​en Debian, es posible que pueda utilizar libeditreadline-dev o
libreadline-gplv2-dev en lugar de libreadline-dev.])
        fi
])

if test "x$HAVE_READLINE" = "xyes"; then
    AC_DEFINE([HAVE_READLINE], [], [Definido a 1 si tiene la biblioteca 'readline' (-lreadline) y los encabezados requeridos])
else
    AC_MSG_ERROR([Falta libreria 'readline'.])
fi
AC_SUBST([HAVE_READLINE])
AC_SUBST([READLINE_LIBS])

##############################################################################
# Section 7 - Language support                                               #
#                                                                            #
# i18n Language support:                                                     #
# see http://cvs.gnome.org/viewcvs/intltool/doc/I18N-HOWTO?rev=1.3           #
##############################################################################

# if you don't have nls tools, allow a way out!

AC_ARG_ENABLE(nls,
    AS_HELP_STRING([--disable-nls], [No usar NLS.]),
              USE_NLS=no, USE_NLS=yes)
AC_SUBST(USE_NLS)

if test "$USE_NLS" = "yes"; then
  AC_CHECK_HEADERS(locale.h)
  AC_CHECK_FUNCS(setlocale)
  AC_SUBST(HAVE_LOCALE_H)

# Add languages here when they are translated.
# Grep the LINGUAS file.
  LANGUAGES=""
  AC_SUBST(LANGUAGES)
  AC_CHECK_HEADERS(libintl.h)
  AC_CHECK_LIB(intl,gettext)
  AC_SUBST(HAVE_LIBINTL_H)
  AC_SUBST(HAVE_LIBINTL)

# The default locale directory is /usr/share/locale - This is generally fixed
# by the distribution....
  dnl Handle localedir
  LOCALEDIR='/usr/share/locale'
  AC_ARG_WITH(locale-dir,
  AS_HELP_STRING(
    [--with-locale-dir=DIR],
    [Localización de archivos locale [DATADIR/locale].]
  ),[
    if test x$withval = xyes; then
      AC_MSG_WARN(El uso es: --with-locale-dir=basedir)
    else
      if test x$withval = xno; then
        AC_MSG_WARN(El uso es: --with-locale-dir=basedir)
      else
        LOCALEDIR=$withval
      fi
    fi
  ])
  AC_SUBST(LOCALEDIR)
fi

#restore LIBS
LIBS="$X_LIBS $temp_LIBS"

AC_PATH_PROG(XGETTEXT,xgettext,none)
XGETTEXT_TCL_WORKS=no
if test "$XGETTEXT" = "none"; then
   AC_MSG_ERROR(instalar gettext 0.14.5 o superior)
fi
AC_SUBST(XGETTEXT)

AC_PATH_PROG(MSGFMT,msgfmt,none)
if test "$MSGFMT" = "none"; then
   AC_MSG_ERROR(instalar gettext 0.14.5 o superior)
fi
AC_SUBST(MSGFMT)

if test "$PYTHON" = "none"; then
    AC_MSG_ERROR([Python no encontrado.  Instálelo o especifique --disable-python para omitir las partes de LinuxCNC que dependen de Python])
fi

# Notificar verificando
AC_MSG_CHECKING([versión python])
if ! $PYTHON -c 'import sys;raise SystemExit(sys.hexversion<0x2070000)'
then
    AC_MSG_ERROR(Versión Python demasiado antigua (se requiere 2.7 o superior))
fi
AC_MSG_RESULT(OK)

# Notificar verificando versiones tk y Tkinter
AC_MSG_CHECKING([coincidencia de versiones tk y Tkinter])
if $PYTHON -c 'import sys;raise SystemExit(sys.hexversion<0x3040000)'
then

	PYTHON_TK_VERSION="`$PYTHON -c 'import tkinter; print(tkinter.TkVersion)'`"
	PYTHON_TCL_VERSION="`$PYTHON -c 'import tkinter; print(tkinter.TclVersion)'`"

else

	PYTHON_TK_VERSION="`$PYTHON -c 'import _tkinter; print(_tkinter.TK_VERSION)'`"
	PYTHON_TCL_VERSION="`$PYTHON -c 'import _tkinter; print(_tkinter.TCL_VERSION)'`"

fi

if test "$PYTHON_TCL_VERSION" != "$TCL_VERSION"; then
    AC_MSG_RESULT([TCL mismatch: $TCL_VERSION vs $PYTHON_TCL_VERSION])
    AC_MSG_ERROR([Python requiere usar Tcl $PYTHON_TCL_VERSION y Tk $PYTHON_TK_VERSION.
Instalar esa versión y especificar --with-tkConfig  y --with-tkConfig si es necesario])
fi
if test "$PYTHON_TK_VERSION" != "$TK_VERSION"; then
    AC_MSG_RESULT([Tk no coincide: $TK_VERSION vs $PYTHON_TK_VERSION])
    AC_MSG_ERROR(["Python requiere usar Tk $TK_VERSION.  Instalar esa versión y especificar --with-tkConfig si es necesario"])
fi
AC_MSG_RESULT([$PYTHON_TK_VERSION])

# Notificar verificando site-package
AC_MSG_CHECKING(para localización site-package)
SITEPY="$($PYTHON -c 'import sysconfig; s = sysconfig.get_scheme_names(); m=list(set(("deb_system", "rpm_prefix")) & set(s)); print(sysconfig.get_path("platlib", m.__getitem__(0))) if m else print("/usr/lib/python3/dist-packages");')"
AC_MSG_RESULT($SITEPY)

# Notificar verificando GLU quadrics
AC_MSG_CHECKING(para trabajo GLU quadrics)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <GL/gl.h>
#include <GL/glu.h>
]], [[GLUquadric *q;]])],[AC_MSG_RESULT(yes)],[
            AC_MSG_ERROR([Falta la libreria GLU.  Instálela o especifique --disable-python para omitir las partes de LinuxCNC que dependen de Python])

])

# Notificar verificando
AC_MSG_CHECKING(para Xmu headers)
AC_CHECK_HEADERS(X11/Xmu/Xmu.h,[],[AC_MSG_ERROR([Falta el encabezado Xmu requerido. Instálelo o especifique --disable-python para omitir las partes de LinuxCNC que dependen de Python])])

AC_SUBST([LIBPYTHON])
AC_SUBST([SITEPY])

AC_SUBST([CFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([CXXFLAGS])
AC_SUBST([LDFLAGS])

##############################################################################
# archivos que se crean mediante ./configure                                 #
# AC_CONFIG_FILES ( archivo …, [cmds], [init-cmds])                          #
# Crea el directorio en el que se encuentra el archivo si no existe.         #
# hay llamadas a comandos para algunos, como chmod.                          #
##############################################################################
#AC_CONFIG_FILES([../configs/rtapi.conf ../configs/emc.conf ../configs/hal.conf config.h])
AC_CONFIG_FILES([../scripts/rtapi.conf])
AC_CONFIG_FILES([../scripts/linuxcnc], [chmod +x ../scripts/linuxcnc])
AC_CONFIG_FILES([../scripts/linuxcnc_info], [chmod +x ../scripts/linuxcnc_info])
AC_CONFIG_FILES([../scripts/halrun], [chmod +x ../scripts/halrun])
AC_CONFIG_FILES([../scripts/rip-environment], [chmod +x ../scripts/rip-environment])
AC_CONFIG_FILES([../scripts/haltcl], [chmod +x ../scripts/haltcl])
AC_CONFIG_FILES([../scripts/halcmd_twopass], [chmod +x ../scripts/halcmd_twopass])
AC_CONFIG_FILES([../scripts/realtime], [chmod +x ../scripts/realtime])
AC_CONFIG_FILES([../scripts/runtests], [chmod +x ../scripts/runtests])
AC_CONFIG_FILES([../scripts/linuxcnc_var], [chmod +x ../scripts/linuxcnc_var])
AC_CONFIG_FILES([../scripts/linuxcnc-checklink], [chmod +x ../scripts/linuxcnc-checklink])
AC_CONFIG_FILES(Makefile.inc)
AC_CONFIG_FILES(Makefile.modinc)
AC_CONFIG_FILES(../tcl/linuxcnc.tcl)
AC_CONFIG_FILES(../lib/python/nf.py)
AC_CONFIG_FILES([../scripts/linuxcncmkdesktop], [chmod +x ../scripts/linuxcncmkdesktop])
AC_CONFIG_FILES(../share/applications/linuxcnc-latency.desktop)
AC_CONFIG_FILES(../share/applications/linuxcnc-latency-histogram.desktop)
AC_CONFIG_FILES(../share/applications/linuxcnc-pncconf.desktop)
AC_CONFIG_FILES(../share/applications/linuxcnc-stepconf.desktop)
AC_CONFIG_FILES(../share/applications/linuxcnc.desktop)
AC_CONFIG_FILES(../share/desktop-directories/linuxcnc-cnc.directory)
AC_CONFIG_FILES(../share/desktop-directories/linuxcnc-ref.directory)
AC_CONFIG_FILES(../share/desktop-directories/linuxcnc-doc.directory)
AC_CONFIG_FILES(../share/menus/CNC.menu)
AC_OUTPUT

##############################################################################
# message to the user what to do next, after a successful ./configure        #
##############################################################################
bold () {
    if tty > /dev/null 2>&1 && type -path tput > /dev/null 2>&1;
    then
        tput smso
    fi
}

offbold () {
    if tty > /dev/null 2>&1 && type -path tput > /dev/null 2>&1;
    then
        tput rmso
    fi
}

echo ""
echo ""
echo "######################################################################"
echo "#           LinuxCNC - Controlador de Máquina Mejorado               #"
echo "######################################################################"
echo "#                                                                    #"
echo "# LinuxCNC es un sistema de software para el control informático de  #"
echo "# máquinas herramientas como fresadoras. LinuxCNC se distribuye bajo #"
echo "# GPL. Visite http://www.linuxcnc.org/ para obtener más detalles.    #"
echo "#                                                                    #"
echo "#                                                                    #"
echo "#       Parece que ./configure se completó correctamente.            #"
echo "#       Esto significa que RT está instalado correctamente.          #"
echo "#       Si las cosas no funcionan, verifique config.log para         #"
echo "#       ver si hay errores o advertencias                            #"
echo "#                                                                    #"
if test "xyes" = "x$RUN_IN_PLACE"; then
echo "#       Luego, compile escribiendo                                   #"
echo "#         make                                                       #"
echo "#         sudo make setuid                                           #"
if test $RTS = uspace; then
echo "# (si requiere comportamiento en tiempo real y acceso al hardware)   #"
fi
echo "#                                                                    #"
echo "#         Antes de ejecutar el software, configure el entorno:       #"
echo "#         . $BUILD_TOPLEVEL/scripts/rip-environment                #"
else
bold
echo "#    advertencia: si ya tiene instalado linuxcnc, esto reemplazará   #"
echo "#          una instalación existente. Si ha instalado                #"
echo "#         un paquete linuxcnc, esto dañará el paquete.               #"
offbold
echo "# sugerencia: para probar una versión de compilación propia de       #"
echo "# linuxcnc sin dañar la versión del paquete,                         #"
echo "# no especifique un --prefix #"
echo "#                                                                    #"
echo "# Luego, compile escribiendo                                         #"
echo "#         make                                                       #"
echo "# luego instálelo escribiendo                                        #"
echo "#         sudo make install                                          #"
fi
echo "#                                                                    #"
echo "# Para ejecutar el software, ejecute                                 #"
echo "# linuxcnc                                                           #"
echo "#                                                                    #"
echo "######################################################################"
echo ""
echo ""

