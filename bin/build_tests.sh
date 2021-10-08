#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..

###### Script #################################################################
echo "~~~ Build All Tests ~~~"
bin/bld_core.sh show_ctx

bin/bld_core.sh unit sanity_tests tests/sanity_tests.c
bin/bld_core.sh unit unicode_test tests/unicode_test.c
bin/bld_core.sh unit cpp_build_test tests/cpp_build_test.cpp
bin/bld_core.sh unit expression_tests tests/expression_tests.c

echo

###### Restore Path ###########################################################
cd $og_path
