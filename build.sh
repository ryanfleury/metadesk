#!/bin/bash

CC=clang

# NOTE(mal): Silent pushd/popd
pushd () {
    command pushd "$@" > /dev/null
}
popd () {
    command popd "$@" > /dev/null
}

echo ~~~ Metadesk Build ~~~
# TODO(mal): Review these warnings
accepted_clang_warnings="-Wno-deprecated-declarations -Wno-pointer-sign -Wno-writable-strings -Wno-unknown-warning-option"
compile_flags="-I../source/ $accepted_clang_warnings"

mkdir -p build
pushd build
echo
echo ~~~ Build All Samples ~~~
$CC $compile_flags ../samples/old_style_custom_layer.c -o old_style_custom_layer
$CC $compile_flags ../samples/static_site_generator/static_site_generator.c -o static_site_generator
$CC $compile_flags ../samples/output_parse/output_parse.c -o output_parse
$CC $compile_flags ../samples/c_code_generation.c -o c_code_generation
$CC $compile_flags ../samples/node_errors/node_errors.c -o node_errors
echo
echo ~~~ Build All Tests ~~~
$CC $compile_flags ../tests/sanity_tests.c -o sanity_tests
$CC $compile_flags ../tests/unicode_test.c -o unicode_test
clang++ $compile_flags ../tests/cpp_build_test.cpp
$CC $compile_flags ../tests/grammar.c -o grammar
popd

echo
echo ~~~ Running Sanity Tests ~~~
pushd build
./sanity_tests
popd

echo
echo ~~~ Running Static Site Generator Sample ~~~
mkdir -p samples/static_site_generator/example_site/generated
pushd samples/static_site_generator/example_site/generated
../../../../build/static_site_generator --siteinfo ../site_info.md --pagedir ../
popd

echo
echo ~~~ Running Output Parse Sample ~~~
mkdir -p samples/output_parse/examples/output
pushd samples/output_parse/examples/output
../../../../build/output_parse ../example.md ../example2.md
popd

echo
echo ~~~ Running C Code Generation Sample ~~~
pushd build
./c_code_generation
popd

echo
echo ~~~ Running Error Generation Sample ~~~
pushd build
./node_errors ../samples/node_errors/node_errors.md
popd

