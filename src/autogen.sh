#!/bin/sh

# Este script:
# - borra recursivo-forzado el direcctorio autom4te.cache
# - los targets del script son dos: configure y config.h.in
# - dos case manejan estos targets usando la variable AUTOGEN_TARGET

# Para el target configure:
#	- busca el directorio libreria de automake (automake --print-libdir). Lo guarda en la var automake_libdir
#	- comprueba existencia, o copia los archivos:
# 1.- config.guess
#		- Intenta adivinar un nombre de sistema canónico.
# 2.- config.sub
#		- El objetivo de este archivo es mapear todas las diversas 
#		variaciones de una especificación de máquina dada en una única especificación en la forma:
#			CPU_TYPE-MANUFACTURER-OPERATING_SYSTEM
#		o en algunos casos, la forma más nueva de cuatro partes:
#			CPU_TYPE-MANUFACTURER-KERNEL-OPERATING_SYSTEM
# 3.-install-sh 
#		- instala un programa, script, o datafile
#
# aclocal
#	- genera 'aclocal.m4' escaneando 'configure.ac'
# Finalmente, autoconf genera el script configure

# Para el target config.h.in:
#	- autoheader crea un archivo de plantilla de declaraciones C `#define' para que `configure' las use.

# Funcion de touch:
#	autoconf solo actualiza la timestamp si la salida realmente cambió.
#	La timestamp del target debe actualizarse o make se confunde

# Nuevos archivos y directorio creados tras autogen.sh
# /autom4te.cache
# aclocal.m4
# config.guess
# config.sub
# configure
# install-sh
# config.h.in

rm -rf autom4te.cache
AUTOGEN_TARGET=${AUTOGEN_TARGET-configure:config.h.in}
set -e
case :$AUTOGEN_TARGET: in
*:configure:*)
    automake_libdir=`automake --print-libdir`
    [ -e config.guess ] || cp $automake_libdir/config.guess .
    [ -e config.sub ] || cp $automake_libdir/config.sub .
    [ -e install-sh ] || cp $automake_libdir/install-sh .
    aclocal --verbose --force
    autoconf -v
    touch configure
    ;;
esac
case :$AUTOGEN_TARGET: in
*:config.h.in:*)
    autoheader -v
    touch config.h.in
    ;;
esac



