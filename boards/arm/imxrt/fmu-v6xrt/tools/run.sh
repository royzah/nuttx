#!/bin/sh
# Checks the CAAM job descriptors on the host, so an encoding typo is found
# here rather than on a board that will not boot.
set -eu
cd "$(dirname "$0")"

CC=${CC:-cc}
ARCH=../../../../../arch/arm/src/imxrt
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

$CC -O1 -g -Wall -Wextra -Wconversion -Wsign-conversion -std=gnu99 \
	-I"$ARCH" -o "$WORK/caam_desc_test" caam_desc_test.c

"$WORK/caam_desc_test"
