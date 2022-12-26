#!/bin/sh

pwd

SRC=$1/../../..
DEST=.
DATADIR=font
LANGDIR=lang

cp -p $SRC/$DATADIR/*.* $DEST
for i in `ls -1 $SRC/$LANGDIR`; do
  if [ -d $SRC/$LANGDIR/$i ]; then
    mkdir -p $DEST/$LANGDIR/$i
    cp -p $SRC/$LANGDIR/$i/*.mo $DEST/$LANGDIR/$i
  fi
done

