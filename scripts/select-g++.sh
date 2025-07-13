#!/usr/bin/env bash

# Select the g++ version to use
if [ ! -z "$GCC_COMPILER" ]; then
    if command -v "$GCC_COMPILER" >/dev/null ; then
        echo -n "$GCC_COMPILER" ;
        exit 0 ;
    fi
fi

for binary in g++-14 g++-13 g++-12 g++-11 g++-10 g++ ; do
    if command -v "$binary" >/dev/null ; then
        echo -n "$binary" ;
        exit 0 ;
    fi
done
