#!/bin/sh

ndkDir=$HOME/Library/Android/sdk/ndk-bundle
toolDir=$ndkDir/build

$toolDir/ndk-build clean -C `dirname $0`/.. NDK_APP_LIBS_OUT=jniLibs NDK_DEBUG=0
