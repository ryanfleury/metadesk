#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..
   root_path=$PWD
  build_path=$root_path/build
samples_path=$root_path/samples

echo ~~~ Running Static Site Generator Sample ~~~
cd $samples_path
if [ -d "static_site_generator/example_site" ]; then
  cd static_site_generator/example_site
  mkdir -p generated
  cd generated
  $build_path/static_site_generator.exe --siteinfo:../site_info.mdesk --pagedir:../
fi
echo

echo ~~~ Running Output Parse Sample ~~~
cd $samples_path
if [ -d "output_parse/examples" ]; then
  cd output_parse/examples
  mkdir -p output
  cd output
  $build_path/output_parse.exe ../example.mdesk ../example2.mdesk
fi
echo

echo ~~~ Running C Code Generation Sample ~~~
cd $build_path
./c_code_generation.exe
echo

echo ~~~ Running Error Generation Sample ~~~
cd $build_path
./node_errors.exe $samples_path/node_errors/node_errors.mdesk
echo

echo ~~~ Running C++ Sample ~~~
cd $build_path
./cpp_build_test.exe
echo

###### Restore Path ###########################################################
cd $og_path
