#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..

mkdir -p build
cd build

echo ~~~ Running Sanity Tests ~~~
./sanity_tests.exe
./unicode_test.exe

echo ~~~ Running Expression Tests ~~~
./expression_tests.exe

###### Restore Path ###########################################################
cd $og_path
