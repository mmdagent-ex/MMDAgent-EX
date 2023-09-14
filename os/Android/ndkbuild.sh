#!/bin/sh

ndkDir=$HOME/Library/Android/sdk/ndk-bundle
toolDir=$ndkDir/build
appDir=$HOME/Android/PocketMMDAgent/app

$toolDir/ndk-build -j4 -C $appDir/src/main NDK_APP_LIBS_OUT=jniLibs NDK_DEBUG=0
