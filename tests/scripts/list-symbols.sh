#!/bin/sh

set -eu

if [ -d include/psa ]; then :; else
    echo "$0: must be run from root" >&2
    exit 1
fi

if grep -i cmake Makefile >/dev/null; then
    echo "$0: not compatible with cmake" >&2
    exit 1
fi

cp include/mbedcrypto/config.h include/mbedcrypto/config.h.bak
scripts/config.pl full
CFLAGS=-fno-asynchronous-unwind-tables make clean lib >/dev/null 2>&1
mv include/mbedcrypto/config.h.bak include/mbedcrypto/config.h
if uname | grep -F Darwin >/dev/null; then
    nm -gUj library/libmbed*.a 2>/dev/null | sed -n -e 's/^_//p'
elif uname | grep -F Linux >/dev/null; then
    nm -og library/libmbed*.a | grep -v '^[^ ]*: *U \|^$\|^[^ ]*:$' | sed 's/^[^ ]* . //'
fi | sort > exported-symbols
make clean

wc -l exported-symbols
