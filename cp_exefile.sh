#!/bin/sh
OS=`uname -s`
if [ "$OS" = "Darwin" ]; then
	mkdir -p ./bin/macosx64
	cp -p ./Release/mpprinter.app/Contents/MacOS/mpprinter ./bin/macosx64/
fi
if [ "$OS" = "Linux" ]; then
	MA=`uname -m`
	if [ "$MA" = "x86_64" ]; then
		DIR=./bin/linux64
	else
		DIR=./bin/linux32
	fi
	mkdir -p $DIR
	cp -p ./Release/mpprinter $DIR
fi
