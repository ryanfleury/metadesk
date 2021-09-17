#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..
    root_path=$PWD
   build_path=$root_path/build
examples_path=$root_path/examples

echo ~~~ Running Error Generation Example ~~~
cd $build_path
./user_errors.exe $examples_path/mdesk_files/user_errors.mdesk
echo

###### Restore Path ###########################################################
cd $og_path
