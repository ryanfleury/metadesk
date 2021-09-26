#!/bin/bash

###### Usage ##################################################################
# ./bld_core.sh <command> [arguments ...]
#
# command: compile
#  ./bld_core.sh compile <source-file> [opts ...]
#  
#  Creates object files from a single source file and options with a compiler.
#  Scans the source file for '//$' and '//' and pulls options out of the tokens
#   in between, combines options from the source file with the passed options.
#
#
# command: link
#  ./bld_core.sh link <out-name> [input-file ...] -- [opts ...]
#
#  Creates binaries (executables and dynamic libraries) from objects,
#   libraries, and options with a linker.
#  First automatically compiles source files into objects.
#  Object files can be specified as '<name>.o' or '<name>.obj', and will be
#   automatically adjusted according to the selected compiler.
#  Library files should be specified with '<name>.lib', and will be
#   automatically adjusted according to the selected linker.
#  If the option 'dll' appears in the list the linker will produce a dynamic
#   library (or "shared object").
#
#
# command: lib
#  ./bld_core.sh lib <out-name> [input-file ...] -- [opts ...]
#  
#  Creates library archives (.lib or .a) from objects.
#  First automatically compiles source files into objects.
#  Object files can be specified as '<name>.o' or '<name>.obj', and will be
#   automatically adjusted according to the selected compiler.


###### Has Opt Function #######################################################

function bld_has_opt {
  ###### parse arguments ####################################################
  local key_opt=$1
  local opts=()
  for ((i=2; i<=$#; i+=1)); do
    opts+=(${!i})
  done
  
  ###### scan ###############################################################
  local has_key=0
  for ((i=0;i<${#opts[@]};i+=1)); do
    local opt=${opts[i]}
    if [[ "$opt" == "$key_opt" ]]; then
      has_key=1
      break
    fi
  done
  
  echo $has_key
}

###### Flags From Opts Function ###############################################

function bld_flags_from_opts {
  ###### parse arguments ####################################################
  local in_file=$1
  local opts=()
  for ((i=2; i<=$#; i+=1)); do
    opts+=(${!i})
  done
  
  ###### load file ##########################################################
  local flags_raw=()
  IFS=$'\r\n' GLOBIGNORE='*' command eval 'flags_raw=($(cat $in_file))'
  
  ###### filter #############################################################
  local flags=()
  for ((i=0;i<${#flags_raw[@]};i+=1)); do
    local flag=${flags_raw[i]}
    
    ###### skip blanks and comments #######################################
    if [[ -z "${flag// }" ]]; then
      continue
    fi
    if [[ "${flag:0:1}" == "#" ]]; then
      continue
    fi
    
    ###### parse line filters #############################################
    local line_filters=()
    while [[ $flag = *">"* ]]; do
      line_filters+=("${flag%%>*}")
      flag="${flag#*>}"
    done
    
    ###### check filters ##################################################
    local can_include=1
    for ((j=0;j<${#line_filters[@]};j+=1)); do
      can_include=0
      for ((k=0;k<${#opts[@]};k+=1)); do
        if [[ ${opts[k]} = ${line_filters[j]} ]]; then
          can_include=1
          break
        fi
      done
      if [[ "$can_include" = "0" ]]; then
        break
      fi
    done
    if [[ "$can_include" = "1" ]]; then
      flags+=("${flag}")
    fi
  done
  
  echo "${flags[@]}"
}

###### Opts From Src Function #################################################

function bld_opts_from_src {
  ###### Split File Into Lines ##############################################
  local in_file=$1
  local tokens=($(grep "//\\$" $in_file))
  
  ###### Parse ##############################################################
  local in_params_range="0"
  local params=()
  for ((i=0; i<${#tokens[@]}; i+=1)); do
    local string="${tokens[i]}"
    if [[ "$in_params_range" == "0" ]]; then
      if [[ "$string" == "//$" ]]; then
        in_params_range="1"
      fi
    elif [[ "$in_params_range" == "1" ]]; then
      if [[ "${string:0:2}" == "//" ]]; then
        break
      fi
      params+=($string)
    fi
  done
  
  echo "${params[@]}"
}

###### Dedup Function #########################################################

function bld_dedup {
  ###### parse arguments ####################################################
  local in=()
  for ((i=1; i<=$#; i+=1)); do
    in+=(${!i})
  done
  
  ###### dedup ##############################################################
  local out=()
  for ((i=0; i<${#in[@]}; i+=1)); do
    local string=${in[i]}
    local is_dup="0"
    for ((j=0; j<${#out[@]}; j+=1)); do
      if [[ "$string" == "${out[j]}" ]]; then
        is_dup="1"
        break
      fi
    done
    if [[ "$is_dup" == "0" ]]; then
      out+=($string)
    fi
  done
  
  echo "${out[@]}"
}

###### Compile Function #######################################################

function bld_compile {
  ###### parse arguments ####################################################
  local in_file=$1
  local opts=()
  for ((i=2; i<=$#; i+=1)); do
      opts+=(${!i})
  done
  
  if [ "$in_file" == "" ]; then
    echo "compile: missing input file"
    return 1
  fi
  
  ###### finith in file #####################################################
  local final_in_file=$root_path/$in_file
  
  ###### out file name ######################################################
  local file_base=${final_in_file##*/}
  local file_base_no_ext=${file_base%.*}
  local out_file="$file_base_no_ext$dot_ext_obj"
  
  ###### finish options #####################################################
  local src_opts=($(bld_opts_from_src $final_in_file))
  local all_opts=($(bld_dedup $file_base ${opts[@]} ${src_opts[@]}))
  
  ###### diagnostics ########################################################
  local diagnostics=$(bld_has_opt diagnostics ${all_opts[@]})
  
  ###### get real flags #####################################################
  local flags_file=$bin_path/compiler_flags.txt
  local flags=($(bld_flags_from_opts $flags_file ${all_opts[@]}))
  
  ###### get inc paths ######################################################
  local paths_file=$bin_path/compiler_inc_paths.txt
  local paths=($(bld_flags_from_opts $paths_file ${all_opts[@]}))
  local incs=()
  for ((i=0; i<${#paths[@]}; i+=1)); do
    incs+=("-I$root_path/${paths[i]}")
  done
  
  ###### move to output folder ##############################################
  mkdir -p "$build_path"
  cd $build_path
  
  ###### delete existing object file ########################################
  rm -f "$out_file_base.o"
  rm -f "$out_file_base.obj"
  
  ###### get flags ##########################################################
  local all_flags="-c -I$src_path ${incs[@]} ${flags[@]}"
  
  ###### diagnostic output ##################################################
  if [ "$diagnostics" == "1" ]; then
    echo "cmp $final_in_file -- ${all_opts[@]}"
    echo $compiler "$final_in_file" $all_flags
  fi
  
  ###### print source file (if the compiler doesn't do it automatically) ####
  if [ "$manually_print_target" == "1" ]; then
    echo "$file_base"
  fi
  
  ###### compile ############################################################
  $compiler "$final_in_file" $all_flags
}

###### Link Function ##########################################################

function bld_link {
  ###### parse arguments ####################################################
  local out_name=$1
  local in_files=()
  for ((i=2; i<=$#; i+=1)); do
    if [ "${!i}" == "--" ]; then
      break
    fi
    in_files+=(${!i})
  done
  
  local opts=()
  for ((i+=1; i<=$#; i+=1)); do
    opts+=(${!i})
  done
  
  if [ "$out_name" == "" ]; then
    echo "link: missing output name"
    return 1
  fi
  if [ "${#in_files}" == "0" ]; then
    echo "link: missing input file(s)"
    return 1
  fi
  
  ###### finish options #####################################################
  local all_opts=($(bld_dedup ${opts[@]}))
  
  ###### diagnostics ########################################################
  local diagnostics=$(bld_has_opt diagnostics ${all_opts[@]})
  
  ###### sort in files ######################################################
  local in_src=()
  local in_obj=()
  local in_lib=()
  for ((i=0; i<${#in_files[@]}; i+=1)); do
    local file="${in_files[i]}"
    local ext="${file##*.}"
    if [[ "$ext" == "c" || "$ext" == "cpp" ]]; then
      in_src+=($file)
    elif [[ "$ext" == "o" || "$ext" == "obj" ]]; then
      in_obj+=($file)
    elif [[ "$ext" == "lib" ]]; then
      in_lib+=($file)
    else
      echo "WARNING: ignoring unrecognized file type $file"
    fi
  done
  
  ###### auto correct object files ##########################################
  for ((i=0; i<${#in_obj[@]}; i+=1)); do
    local file_name="${in_obj[i]}"
    local base_name="${file_name%.*}"
    in_obj[$i]="$base_name$dot_ext_obj"
  done
  
  ###### compile source files ###############################################
  for ((i=0; i<${#in_src[@]}; i+=1)); do
    bld_compile "${in_src[i]}" ${all_opts[@]}
    status=$?
    if [ $status -ne 0 ]; then
      exit $status
    fi
  done
  
  ###### intermediate object files ##########################################
  local interm_obj=()
  for ((i=0; i<${#in_src[@]}; i+=1)); do
    local file_name="${in_src[i]}"
    local base_name="${file_name##*/}"
    local base_name_no_ext="${base_name%.*}"
    interm_obj+=($base_name_no_ext$dot_ext_obj)
  done
  
  ###### get real flags #####################################################
  local flags_file=$bin_path/linker_flags.txt
  local flags=$(bld_flags_from_opts $flags_file ${all_opts[@]})
  
  ###### link ###############################################################
  local linker_kind="link"
  if [ $linker == "clang" ]; then
    linker_kind="clang"
  fi
  
  ###### out file name ######################################################
  local bin_kind="exe"
  for ((i=0; i<${#all_opts[@]}; i+=1)); do
    if [[ "${all_opts[i]}" == "dll" ]]; then
      bin_kind="dll"
      break
    fi
  done
  
  dot_ext_out="$dot_ext_exe"
  if [ "$bin_kind" == "dll" ]; then
    dot_ext_out="$dot_ext_dll"
  fi
  out_file="$out_name$dot_ext_out"
  
  ###### move to output folder ##############################################
  mkdir -p "$build_path"
  cd $build_path
  
  ###### diagnostic output ##################################################
  local final_in_files="${interm_obj[@]} ${in_obj[@]} ${in_lib[@]}"
  if [ "$diagnostics" == "1" ]; then
    echo "lnk $final_in_files -- ${all_opts[@]}"
  fi
  
  ###### link ###############################################################
  if [ "$linker_kind" == "link" ]; then
    if [ "$diagnostics" == "1" ]; then
      echo $linker -OUT:"$out_file" $flags $final_in_files
    fi
    echo "$out_file"
    $linker -OUT:"$out_file" $flags $final_in_files
  elif [ "$linker_kind" == "clang" ]; then
    if [ "$diagnostics" == "1" ]; then
      echo $linker -o "$out_file" $flags $final_in_files
    fi
    echo "$out_file"
    $linker -o "$out_file" $flags $final_in_files
  else
    echo "ERROR: invokation not defined for this linker"
    false
  fi
  status=$?
  
  return $status
}

###### Library Function #######################################################

function bld_lib {
  ###### parse arguments ####################################################
  local out_name=$1
  local in_files=()
  for ((i=2; i<=$#; i+=1)); do
    if [ "${!i}" == "--" ]; then
      break
    fi
    in_files+=(${!i})
  done
  
  local opts=()
  for ((i+=1; i<=$#; i+=1)); do
    opts+=(${!i})
  done
  
  if [ "$out_name" == "" ]; then
    echo "lib: missing output name"
    return 1
  fi
  if [ "${#in_files}" == "0" ]; then
    echo "lib: missing input file(s)"
    return 1
  fi
  
  ###### finish options #####################################################
  local all_opts=($(bld_dedup ${opts[@]}))
  
  ###### diagnostics ########################################################
  local diagnostics=$(bld_has_opt diagnostics ${all_opts[@]})
  
  ###### sort in files ######################################################
  local in_src=()
  local in_obj=()
  for ((i=0; i<${#in_files[@]}; i+=1)); do
    local file="${in_files[i]}"
    local ext="${file##*.}"
    if [[ "$ext" == "c" || "$ext" == "cpp" ]]; then
      in_src+=($file)
    elif [[ "$ext" == "o" || "$ext" == "obj" ]]; then
      in_obj+=($file)
    else
      echo "WARNING: ingnoring unrecgonized file type $file"
    fi
  done
  
  ###### auto correct object files ##########################################
  for ((i=0; i<${#in_obj[@]}; i+=1)); do
    local file_name="${in_obj[i]}"
    local base_name="${file_name%.*}"
    in_obj[$i]=$base_name$dot_ext_obj
  done
  
  ###### compile source files ###############################################
  for ((i=0; i<${#in_src[@]}; i+=1)); do
    compile "${in_src[i]}" ${all_opts[@]}
    local status=$?
    if [ $status -ne 0 ]; then
      return $status
    fi
  done
  
  ###### intermediate object files ##########################################
  local interm_obj=()
  for ((i=0; i<${#in_src[@]}; i+=1)); do
    local file_name="${in_src[i]}"
    local base_name="${file_name##*/}"
    local base_name_no_ext="${base_name%.*}"
    interm_obj+=($base_name_no_ext$dot_ext_obj)
  done
  
  ###### out file name ######################################################
  local out_file=""
  if [ "$os" == "windows" ]; then
    out_file="$out_name.lib"
  elif [ "$os" == "linux" || "$os" == "mac" ]; then
    out_file="lib$out_name.a"
  else
    echo "ERROR: static library output not defined for OS: $os"
  fi
  
  ###### diagnostic output ##################################################
  local final_in_files="${interm_obj[@]} ${in_obj[@]}"
  if [ "$diagnostics" == "1" ]; then
    echo "lib $final_in_files -- ${all_opts[@]}"
  fi
  
  ###### print output file ##################################################
  echo "$out_file"
  
  ###### move to output folder ##############################################
  mkdir -p "$build_path"
  cd $build_path
  
  ###### build library ######################################################
  if [ "$os" == "windows" ]; then
    lib -nologo -OUT:"$out_file" $final_in_files
  elif [ "$os" == "linux" || "$os" == "mac" ]; then
    # TODO(allen): invoke ar here - make sure to delete the original .a first
    # because ar does not (seem) to replace the output file, just append
    echo "TODO: implement ar path in bld_core.sh:bld_lib"
    false
  else
    echo "ERROR: static library invokation not defined for OS: $os"
    false
  fi
  status=$?
  
  return $status
}

###### Unit Function ##########################################################

function bld_unit {
  ###### parse arguments ####################################################
  local out_name=$1
  local main_file=$2
  in_files=()
  for ((i=3; i<=$#; i+=1)); do
      if [ "${!i}" == "--" ]; then
          break
      fi
      in_files+=(${!i})
  done
  
  if [ "$out_name" == "" ]; then
    echo "unit: missing output name"
    return 1
  fi
  if [ "$main_file" == "" ]; then
    echo "unit: missing main file"
    return 1
  fi
  
  ###### finish options #####################################################
  local src_opts=$(bld_opts_from_src $root_path/$main_file)
  local all_opts=($(bld_dedup $out_name ${src_opts[@]} ${implicit_opts[@]}))
  
  ###### link ###############################################################
  bld_link $out_name $main_file ${in_files[@]} -- ${all_opts[@]}
  status=$?
  
  return $status
}


###### Show Ctx Function ######################################################

function bld_show_ctx {
  local mod_opts=()
  for ((i=0;i<${#implicit_opts[@]};i+=1)); do
    local mod_opt="[${implicit_opts[i]}]"
    mod_opts+=($mod_opt)
  done
  echo "${mod_opts[@]}"
}


###### Get Paths ##############################################################
og_path=$PWD
cd "$(dirname "$0")"
cd ..

 root_path=$PWD
  bin_path="$root_path/bin"
local_path="$root_path/local"
build_path="$root_path/build"
  src_path="$root_path/source"


###### Crack Operating System From Environment ################################
os="undefined"
if [ "$OSTYPE" == "win32" ] ||
   [ "$OSTYPE" == "msys"  ]; then
  os="windows"
elif [ "$OSTYPE" == "linux-gnu" ]; then
  os="linux"
elif [ "$OSTYPE" == "darwin" ]; then
  os="osx"
fi


###### Implicit Options #######################################################
    compiler=$($local_path/compiler.sh)
      linker=$($local_path/linker.sh)
compile_mode=$($local_path/compile_mode.sh)
        arch=$($local_path/arch.sh)
         ctx=$($local_path/ctx.sh)


###### Apply Override #########################################################
parse_i="1"
override_list=()
if [ "${!parse_i}" == "override" ]; then
  for ((parse_i+=1; parse_i<=$#; parse_i+=1)); do
    arg="${!parse_i}"
    if [ "$arg" == "--" ]; then
      ((parse_i+=1))
      break
    fi
    if [[ "$arg" == *":"* ]]; then
      arg_key="${arg%%:*}"
      arg_val="${arg#*:}"
      declare "${arg_key}=$arg_val"
      override_list+=("[$arg]")
    fi
  done
fi


###### Finish Implicit Options ################################################
implicit_opts=($out_name $compiler $linker $compile_mode $os $arch $ctx)


###### Object File Extension ##################################################
dot_ext_obj=".o"
if [[ "$compiler" == "cl" ]]; then
  dot_ext_obj=".obj"
fi


###### Binary File Extension ##################################################
dot_ext_exe=""
dot_ext_dll=""
if [ "$os" == "windows" ]; then
  dot_ext_exe=".exe"
  dot_ext_dll=".dll"
elif [ "$os" == "linux" ] || [ "$os" == "mac" ]; then
  dot_ext_exe=""
  dot_ext_dll=".so"
else
  echo "ERROR: binary extension not defined for OS: $os"
fi


###### Binary File Extension ##################################################
manually_print_target="0"
if [ "$compiler" == "clang" ]; then
  manually_print_target="1"
fi


###### Diagnostics ############################################################
top_diagnostics=$(bld_has_opt diagnostics ${implicit_opts[@]})


###### Overrides Diagnostics ##################################################
if [[ "$top_diagnostics" == "1" && "${#override_list}" != "0" ]]; then
  echo "${override_list[@]} {"
fi


###### Control ################################################################
command=${!parse_i}
args=()
for ((parse_i+=1; parse_i<=$#; parse_i+=1)); do
  args+=(${!parse_i})
done

if [ "$command" == "compile" ]; then
  bld_compile ${args[@]}
elif [ "$command" == "link" ]; then
  bld_link ${args[@]}
elif [ "$command" == "lib" ]; then
  bld_lib ${args[@]}
elif [ "$command" == "unit" ]; then
  bld_unit ${args[@]}
elif [ "$command" == "show_ctx" ]; then
  bld_show_ctx
else
  echo "'$command' not a recognized command"
fi


###### Overrides Diagnostics ##################################################
if [[ "$top_diagnostics" == "1" && "${#override_list}" != "0" ]]; then
  echo "}"
fi


###### Restore Path ###########################################################
cd $og_path
