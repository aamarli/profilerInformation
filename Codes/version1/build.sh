#!/bin/env bash

die () {
  echo "ERROR: $@"
  exit -1
}

module_load () {
  local _module=$1
  module load $_module
  module -t list |& grep $_module
}

BIN_DIR=`dirname $0`
BLD_DIR=$BIN_DIR/build

if ! module_load cmake ; then die "CMake did not load" ; fi
if ! module_load rocm ; then die "ROCm did not load" ; fi

export ROCM_PATH=${ROCM_PATH:=/opt/rocm-6.0.0}
export CMAKE_PREFIX_PATH=${ROCM_PATH}/include/hsa:${ROCM_PATH}
rm -rf $BLD_DIR result* &&\
 mkdir $BLD_DIR && \
 cd $BLD_DIR && \
 cmake ../. && \
 make -j
 
#sed -i -e 's/$/-pthread -lldms -lldmsd_stream/g' CMakeFiles/rocp-tool.dir/link.txt \
