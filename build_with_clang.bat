@echo off

echo ~~~ Metadesk Build ~~~
set src=%cd%

rem TODO(mal): Review these warnings
set accepted_clang_warnings=-Wno-deprecated-declarations -Wno-pointer-sign -Wno-writable-strings -Wno-unknown-warning-option
set compile_flags=-I%src%/source/ %accepted_clang_warnings%

if not exist build mkdir build
pushd build
echo.
echo ~~~ Build All Samples ~~~
clang %compile_flags% %src%\samples\old_style_custom_layer.c -o old_style_custom_layer.exe
clang %compile_flags% %src%\samples\static_site_generator\static_site_generator.c -o static_site_generator.exe
clang %compile_flags% %src%\samples\output_parse\output_parse.c -o output_parse.exe
clang %compile_flags% %src%\samples\c_code_generation.c -o c_code_generation.exe
clang %compile_flags% %src%\samples\node_errors\node_errors.c -o node_errors.exe
echo.
echo ~~~ Build All Tests ~~~
clang %compile_flags% %src%\tests\sanity_tests.c -o sanity_tests.exe
clang %compile_flags% %src%\tests\unicode_test.c -o unicode_test.exe
clang++ %compile_flags% %src%\tests\cpp_build_test.cpp -o cpp_build_test.exe
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
..\..\..\..\build\static_site_generator.exe --siteinfo ..\site_info.mdesk --pagedir ..\
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
..\..\..\..\build\output_parse.exe ..\example.mdesk ..\example2.mdesk
popd
popd
popd
popd

echo.
echo ~~~ Running C Code Generation Sample ~~~
pushd build
c_code_generation.exe
popd

echo.
echo ~~~ Running Error Generation Sample ~~~
pushd build
node_errors.exe %~dp0\samples\node_errors\node_errors.mdesk
popd


echo.
echo ~~~ Running C++ Sample ~~~
pushd build
cpp_build_test.exe
popd

