#!/bin/sh
rm -f headers.tar
cd src
tar cf ../headers.tar include
cd ..
tar xf headers.tar
rm -f headers.tar
