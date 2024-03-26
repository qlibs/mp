#!/usr/bin/env bash
#
# Copyright Bruno Dutra 2016
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if [[ $# != 1 ]]
then
    echo "Usage:"
    echo "  $0 <URL>"
    exit 1
fi

curl -ILfSs $1 > /dev/null || exit 2

curl -Ls $1 \
    | perl \
        -pe 's/(.{2048}[^ ,;]+.)/\1\n/g;' \
        -pe 's/\\/\\\\/g;' \
        -pe 's/"/\\"/g;' \
        -pe 's/@/\\@/g;' \
        -pe 's/\n/\\\n/g;' \
    || exit 3
