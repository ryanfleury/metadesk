#!/bin/bash

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..

###### Script #################################################################
echo "~~~ Build All Exampes ~~~"
bin/bld_core.sh show_ctx

metasrc="examples/metaprograms"

bin/bld_core.sh unit datadesk_like $metasrc/datadesk_like_template.c
bin/bld_core.sh unit node_errors   $metasrc/node_errors.c
bin/bld_core.sh unit parse_check   $metasrc/parse_check.c

echo

###### Restore Path ###########################################################
cd $og_path
