#!/usr/bin/env bash

set -e

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BUILDDIR="$SCRIPTDIR/cmake-build-fuzz"
TESTCASEDIR="$SCRIPTDIR/../tests/testcases"
CORPUSDIR="$SCRIPTDIR/corpus"

mkdir -p "$CORPUSDIR"
cp "$TESTCASEDIR"/*.raw "$CORPUSDIR"

"$BUILDDIR"/fuzz_message "$CORPUSDIR" > /dev/null
