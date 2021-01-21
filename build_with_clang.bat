@echo off

echo ~~~ Metadesk Build ~~~
set accepted_clang_warnings=-Wno-return-type -Wno-deprecated-declarations -Wno-pointer-sign -Wno-writable-strings -Wno-unknown-warning-option
set compile_flags=-I../source/ %accepted_clang_warnings%

if not exist build mkdir build
pushd build
echo.
echo ~~~ Build All Samples ~~~
clang %compile_flags% ..\samples\old_style_custom_layer.c -o old_style_custom_layer.exe
clang %compile_flags% ..\samples\static_site_generator\static_site_generator.c -o static_site_generator.exe
clang %compile_flags% ..\samples\output_parse\output_parse.c -o output_parse.exe
echo.
echo ~~~ Build All Tests ~~~
clang %compile_flags% ..\tests\sanity_tests.c -o sanity_tests.exe
clang %compile_flags% ..\tests\unicode_test.c -o unicode_test.exe
clang++ %compile_flags% ..\tests\cpp_build_test.cpp -o cpp_build_test.exe
popd

echo.
echo ~~~ Running Sanity Tests ~~~
pushd build
sanity_tests.exe
popd

echo.
echo ~~~ Running Static Site Generator Sample ~~~
pushd samples
pushd static_site_generator
pushd example_site
if not exist generated mkdir generated
pushd generated
..\..\..\..\build\static_site_generator.exe --siteinfo ..\site_info.md --pagedir ..\
popd
popd
popd
popd

echo.
echo ~~~ Running Output Parse Sample ~~~
pushd samples
pushd output_parse
pushd examples
if not exist output mkdir output
pushd output
..\..\..\..\build\output_parse.exe ..\example.md ..\example2.md
popd
popd
popd
popd
