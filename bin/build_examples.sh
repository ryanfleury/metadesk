#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..

###### Script #################################################################
echo "~~~ Build All Examples ~~~"
bin/bld_core.sh show_ctx

examps="examples"

bin/bld_core.sh unit hello_world    $examps/intro/hello_world.c
bin/bld_core.sh unit parse_check    $examps/intro/parse_check.c
bin/bld_core.sh unit data_desk_like $examps/intro/data_desk_like_template.c
bin/bld_core.sh unit user_errors    $examps/user_errors/user_errors.c

bin/bld_core.sh unit type_metadata $examps/type_metadata/type_metadata.c
if [ -d $examps/type_metadata/generated/type_info_meta.h ]; then
bin/bld_core.sh unit type_info     $examps/type_metadata/type_info_final_program.c
fi

bin/bld_core.sh unit expr_intro    $examps/expr/expr_intro.c
bin/bld_core.sh unit expr_c_like   $examps/expr/expr_c_like.c

bin/bld_core.sh unit overrides          $examps/integration/overrides.c

bin/bld_core.sh unit multi_threaded     $examps/integration/multi_threaded.c
bin/bld_core.sh unit memory_management  $examps/integration/memory_management.c

echo

###### Restore Path ###########################################################
cd $og_path
