#!/bin/sh

# Abort on errors (and uninitialised variables)
set -eu

if [ -d library -a -d include -a -d tests ]; then :; else
    echo "Must be run from mbed TLS root" >&2
    exit 1
fi

ls tests/data_files | \
    while read f; do
        grep -R -F --exclude=Makefile "$f" tests >/dev/null || echo "$f"
    done
