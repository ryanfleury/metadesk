#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..

echo ~~~ Running Sanity Tests ~~~
mkdir -p build
cd build
./sanity_tests.exe
./unicode_test.exe

###### Restore Path ###########################################################
cd $og_path
