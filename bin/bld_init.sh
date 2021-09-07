#!/bin/bash

###### Usage ##################################################################
# ./bld_init.sh
# Clears out the local folder and populates it with default initial scripts.

##### Declare The Local File Names ############################################
local_file_names=(
  compiler.sh
  compile_mode.sh
  ctx.sh
  linker.sh
  arch.sh
)

###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..
 root_path=$PWD
  bin_path="$root_path/bin"
local_path="$root_path/local"
 defs_path="$bin_path/local_defaults"


###### Copy Path ##############################################################
rm -rf $local_path
mkdir -p $local_path

for ((i=0; i<${#local_file_names[@]}; i+=1)); do
  file_name=${local_file_names[i]}
  cp $defs_path/default_$file_name $local_path/$file_name
done

###### Restore Path ###########################################################
cd $og_path
