#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..
    root_path=$PWD
   build_path=$root_path/build
examples_path=$root_path/examples

echo ~~~ Running Output Parse Example ~~~
cd $examples_path
if [ -d "output_parse/examples" ]; then
  cd output_parse/examples
  mkdir -p output
  cd output
  $build_path/output_parse.exe ../example.mdesk ../example2.mdesk
fi
echo

echo ~~~ Running Error Generation Example ~~~
cd $build_path
./node_errors.exe $examples_path/node_errors/node_errors.mdesk
echo

echo ~~~ Running C++ Example ~~~
cd $build_path
./cpp_build_test.exe
echo

###### Restore Path ###########################################################
cd $og_path
