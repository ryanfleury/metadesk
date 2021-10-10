#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..
root_path=$PWD
build_path=$root_path/build
examps=$root_path/examples

echo ~~~ Running Memory Management Example ~~~
$build_path/memory_management.exe $examps/intro/hello_world.mdesk $examps/intro/labels.mdesk $examps/intro/sets.mdesk
echo

echo ~~~ Running Multi-Threaded Parse Example ~~~
$build_path/multi_threaded.exe $examps/intro/hello_world.mdesk $examps/intro/labels.mdesk $examps/intro/sets.mdesk
echo

echo ~~~ Running Type Metadata Generator Example ~~~
cd $examps/type_metadata/generated
$build_path/type_metadata.exe $examps/type_metadata/types.mdesk
echo

echo ~~~ Running Error Generation Example ~~~
$build_path/user_errors.exe $examps/user_errors/user_errors.mdesk
echo

echo ~~~ Running Expression Intro ~~~
$build_path/expr_intro.exe $examps/expr/expr_intro.mdesk
echo

echo ~~~ Running C Like Expression ~~~
$build_path/expr_c_like.exe $examps/expr/expr_c_like.mdesk
echo

###### Restore Path ###########################################################
cd $og_path
