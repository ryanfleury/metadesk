#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..
root_path=$PWD
build_path=$root_path/build
examps=$root_path/examples

echo ~~~ Running Expression Intro ~~~
$build_path/expr_intro.exe $examps/expr/expr_intro.mdesk
echo

exit

# Setup a big list of files for a few of the examples
big_list=()
big_list+=("$examps/intro/hello_world.mdesk")
big_list+=("$examps/intro/labels.mdesk")
big_list+=("$examps/intro/sets.mdesk")
big_list+=("$examps/type_metadata/types.mdesk")
big_list+=("$examps/type_metadata/bad_types.mdesk")
big_list+=("$examps/expr/expr_intro.mdesk")
big_list+=("$examps/expr/expr_c_like.mdesk")

echo ~~~ Running Type Metadata Generator Example ~~~
cd $examps/type_metadata/generated
$build_path/type_metadata.exe $examps/type_metadata/types.mdesk
echo

echo ~~~ Running Error Generation Example ~~~
$build_path/user_errors.exe $examps/user_errors/user_errors.mdesk
echo

echo ~~~ Running C Like Expression ~~~
$build_path/expr_c_like.exe $examps/expr/expr_c_like.mdesk
echo

echo ~~~ Running Multi-Threaded Parse Example ~~~
$build_path/multi_threaded.exe ${big_list[@]}
echo

echo ~~~ Running Memory Management Example ~~~
$build_path/memory_management.exe ${big_list[@]}
echo

###### Restore Path ###########################################################
cd $og_path
