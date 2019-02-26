#!/bin/sh

set -eu

if [ -d include/mbedtls ]; then :; else
    echo "$0: must be run from root" >&2
    exit 1
fi

HEADERS=$( ls include/mbedtls/*.h include/psa/*.h | egrep -v 'compat-1\.3\.h' )

# White-list macros we want to be able to refer to that don't exist in the
# crypto library, useful when referring to macros in Mbed TLS from comments.
WHITELIST='MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS'

sed -n -e 's/.*#define \([a-zA-Z0-9_]*\).*/\1/p' $HEADERS \
    | egrep -v '^(asm|inline|EMIT|_CRT_SECURE_NO_DEPRECATE)$|^MULADDC_' \
    | sort -u > macros

# Add the white-listed entries in sorted order.
echo $WHITELIST >> macros
sort -u -o macros macros

wc -l macros
