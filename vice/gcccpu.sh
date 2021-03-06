#!/bin/sh
# gcccpu.sh helper
#
# written by Marco van den Heuvel <blackystardust68@yahoo.com>
#            Marcus Sutton <loggedoubt@gmail.com>
#
# This script compiles certain *cpu.c files with -O0 or -O1

cpufile=no
cpuopt=no
oldopt=no
filename=no

for i in $*
do
  case "$i" in
    *c64cpu.c|*c64dtvcpu.c|*z80.c)
      cpufile=yes
      cpuopt=-O1
      ;;
    *scpu64cpu.c)
      cpufile=yes
      cpuopt=-O0
      ;;
  esac

  case "$i" in
    -O2|-O3)
      oldopt=$i
      ;;
  esac

  case "$i" in
    *.c|*.cc)
      filename=$i
      ;;
  esac
done

newcmdline="gcc"

if test x"$filename" != "xno"; then
  if test x"$cpufile" = "xyes"; then
    echo "compiling $filename with $cpuopt"
  elif test x"$oldopt" != "xno"; then
    echo "compiling $filename with $oldopt"
  else
    echo "compiling $filename normally"
  fi
fi

for i in $*
do
  isopt=no
  case "$i" in
    -O2|-O3)
      isopt=yes
      ;;
  esac

  if test x"$cpufile" = "xyes" -a x"$isopt" = "xyes"; then
    newcmdline="$newcmdline $cpuopt"
  else
    newcmdline="$newcmdline $i"
  fi
done

$newcmdline
