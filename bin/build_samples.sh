#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..

###### Script #################################################################
echo "~~~ Build All Samples ~~~"
bin/bld_core.sh show_ctx

bin/bld_core.sh unit old_style_custom_layer samples/old_style_custom_layer.c
bin/bld_core.sh unit toy_language samples/toy_language/toy_language.c
bin/bld_core.sh unit static_site_generator samples/static_site_generator/static_site_generator.c
bin/bld_core.sh unit output_parse samples/output_parse/output_parse.c
bin/bld_core.sh unit c_code_generation samples/c_code_generation.c
bin/bld_core.sh unit node_errors samples/node_errors/node_errors.c
bin/bld_core.sh unit parse_check samples/parse_check.c

echo

###### Restore Path ###########################################################
cd $og_path
