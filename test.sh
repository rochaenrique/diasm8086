#!/bin/bash

NUM=`printf %04d $1`
EXPECTED=$(find listings -regex "listings/listing_${NUM}_[a-z_]*")
echo Testing against: $EXPECTED

TMP="test_${NUM}"
echo Dumping into $TMP
./diasm8086 $EXPECTED > "$TMP.asm"

echo Compiling with NASM
nasm $TMP.asm

echo Diffing
diff $TMP $EXPECTED
RESUL=$?

echo Removing tmp files
rm $TMP.asm $TMP

exit $RESUL
