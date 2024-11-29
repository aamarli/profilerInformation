#!/bin/env bash
rocm_version=${rocm_version:='6.2.0'}
rm -Rf rocprofiler-sdk-*
snl_proxy_setup () {
  proxy="http://user:pass@proxy.sandia.gov:80"
  for _protocol in \
    "all_proxy" \
    "ftp_proxy" \
    "http_proxy" \
    "https_proxy" \
    "rsync_proxy" \
    "socks_proxy" \
    "ALL_PROXY" \
    "FTP_PROXY" \
    "HTTP_PROXY" \
    "HTTPS_PROXY" \
    "RSYNC_PROXY" \
    "SOCKS_PROXY"
  do
    export $_protocol="$proxy"
  done
  noproxy="web.sandia.gov,srn.sandia.gov,sandia.gov,*.web.sandia.gov,*.srn.sandia.gov,*.sandia.gov,localhost"
  for _noproxy in \
    "no_proxy" \
    "NO_PROXY"
  do
    export "${_noproxy}"="$noproxy"
  done
}

## capture the steps required to reproducibly build rocprofiler-sdk from source
here_spackenv() {
cat << HERE_SPACK_ENV >spack.yaml
# This is a Spack Environment file.
#
# It describes a set of packages to be installed, along with
# configuration settings.
spack:
  specs:
  - cmake
  - git
  - rocm-core@${rocm_version}
  - hip@${rocm_version}
  - aqlprofile@${rocm_version}
  - rocprofiler-dev
  - py-cppheaderparser
  concretizer:
    unify: when_possible
  view:
    rocprof_gnu:
      root: \$spack/../rocprof_gnu
      link: roots
  compilers:
  - compiler:
      spec: gcc@=9.2.0
      paths:
        cc: /home/projects/x86-64/gcc/9.2.0/bin/gcc
        cxx: /home/projects/x86-64/gcc/9.2.0/bin/g++
        f77: /home/projects/x86-64/gcc/9.2.0/bin/gfortran
        fc: /home/projects/x86-64/gcc/9.2.0/bin/gfortran
      flags: {}
      operating_system: rocky8
      target: x86_64
      modules: []
      environment: {}
      extra_rpaths: []

  modules:
    default:
      roots:
        tcl: \$spack/share/spack/modules
        lmod: \$spack/share/spack/lmod
      enable:
      - tcl

      tcl:
        all:
          autoload: direct

    rocprof_gnu:
      roots:
        tcl: \$spack/share/spack/modules
        lmod: \$spack/share/spack/lmod
      use_view: rocprof_gnu
      enable:
      - tcl
      tcl:
        all:
          autoload: direct
HERE_SPACK_ENV
}

ml_test() {
  local _mf=$1
  module load ${_mf}
  module list |& grep ${_mf}
}

message () {
    local color
    local OPTIND
    local opt
    while getopts "crgymn" opt; do
        case $opt in
            c)  color=$(tput setaf 6) ;;
            r)  color=$(tput setaf 1) ;;
	    g)  color=$(tput setaf 2) ;;
	    y)  color=$(tput setaf 3) ;;
	    m)  color=$(tput setaf 5) ;;
            *)  color=$(tput sgr0)    ;;
        esac
    done
    shift $(($OPTIND -1))
    printf "${color}%-10s %-50s %-50s %-50s\n" "$1" "$2" "$3" "
$4"
    tput sgr0
}  

die () {
 message -r "ERROR" "$@" >&2
 exit 2
}

spack_env_build () {
  SPACK_COMMIT_HASH=${SPACK_COMMIT_HASH:="482e2fbde88c1f0fe9c05fd066c9cd70054c7196"}
  if ! [ -d myspack ] ; then
    git clone http://github.com/spack/spack.git -b develop myspack ||\
      die "Spack github clone failed"
  fi
  SPACK_ROOT=$(readlink -f myspack)
  pushd $SPACK_ROOT &>>/dev/null
  git checkout ${SPACK_COMMIT_HASH}
  popd &>>/dev/null
  module purge
  ml_test gcc/9.2.0
  source $SPACK_ROOT/share/spack/setup-env.sh
  spack env create rocprof_gnu
  spack env activate rocprof_gnu
  [ -d /projects/AMD_GPU_SAMPLER/spack_build_cache/build_cache ] &&\
    spack mirror add rocprof_cache /projects/AMD_GPU_SAMPLER/spack_build_cache
  spack mirror list |& grep rocprof_cache || die "rocprof_cache was not added"
  spack mirror set --autopush --unsigned --type binary rocprof_cache
  pushd $SPACK_ROOT/var/spack/environments/rocprof_gnu &>/dev/null
  here_spackenv
  popd &>/dev/null
  spack concretize -fU
  spack install -y --use-buildcache never --no-check-signature
  spack buildcache push -fu --update-index --with-build-dependencies /projects/AMD_GPU_SAMPLER/spack_build_cache
  spack module tcl refresh -y
}

snl_proxy_setup
## THESE ARE TEMPORARY
#  SPACK_ROOT=$(readlink -f myspack)
#  source $SPACK_ROOT/share/spack/setup-env.sh
#  spack env activate rocprof_gnu
#  spack module tcl refresh -y
## ^^^ THESE ARE TEMPORARY
START=${PWD}
spack_env_build
module purge
if [ -f load_modules.sh ] ; then rm load_modules.sh ; fi
for package in "rocm-core" "cmake" "git" "hip@${rocm_version}" "aqlprofile@${rocm_version}" "rocprofiler-dev" "py-cppheaderparser" ; do
  spack module tcl loads --dependencies $package >>load_modules.sh
done
sort load_modules.sh | uniq >spack_modules.sh

[ -f spack_modules.sh ] && source spack_modules.sh || echo "cannot source $PWD/spack_modules.sh"
module --no-pager -t list
command -v cmake &>/dev/null || die "cmake didn't successfully load"
[ -d rocprofiler-sdk-source ] && rm -Rf rocprofiler-sdk-source
git clone https://github.com/ROCm/rocprofiler-sdk.git -b rocm-${rocm_version} rocprofiler-sdk-source
# perfetto's origin and LC fork are not accessible to us
pushd rocprofiler-sdk-source &>/dev/null
git submodule set-url external/perfetto https://github.com/jennfshr/perfetto.git
git submodule set-branch external/perfetto master
popd &>/dev/null
elf_root=$(spack find -p elfutils | tail -n1 | grep elfutils | awk '{print $NF}' | sed 's/ //g')
message -c "elf_root: $elf_root"
export ELF_ROOT=${elf_root}
CPATH+=":${ELF_ROOT}/include"
CPPFLAGS+=" -I ${ELF_ROOT}/include "
CMAKE_PREFIX_PATH+=";${ELF_ROOT}"
CMAKE_EXE_LINKER_FLAGS+=" -L${ELF_ROOT}/lib -ldw "
LDFLAGS+=" -L${ELF_ROOT}/lib -ldw "
LD_LIBRARY_PATH="${ELF_ROOT}/lib:${LD_LIBRARY_PATH}"
hsakml_roct_root=$(spack find -p hsakmt-roct | tail -n1 | grep hsakmt-roct | awk '{print $NF}' | sed 's/ //g')
message -c "hsakml_roct_root: $hsakml_roct_root"
export HSAKML_ROCT_ROOT=$hsakml_roct_root
CPATH+=":${HSAKML_ROCT_ROOT}/include"
CPPFLAGS+=" -I ${HSAKML_ROCT_ROOT}/include " 
CMAKE_PREFIX_PATH+=";${HSAKML_ROCT_ROOT}"
CMAKE_EXE_LINKER_FLAGS+=" -L${HSAKML_ROCT_ROOT}/lib "
LDFLAGS+=" -L${HSAKML_ROCT_ROOT}/lib "
LD_LIBRARY_PATH="${HSAKML_ROCT_ROOT}/lib:${LD_LIBRARY_PATH}"
comgr_root=$(spack find -p comgr | tail -n1 | grep comgr | awk '{print $NF}'| sed 's/ //g')
message -c "comgr_root: $comgr_root"
export COMGR_ROOT=$comgr_root
CPATH+=":${COMGR_ROOT}/include"
CPPFLAGS+=" -I ${COMGR_ROOT}/include "
CMAKE_PREFIX_PATH+=";${COMGR_ROOT}"
CMAKE_EXE_LINKER_FLAGS+=" -L${COMGR_ROOT}/lib -lamd_comgr "
LDFLAGS+=" -L${COMGR_ROOT}/lib -lamd_comgr " 
LD_LIBRARY_PATH="${COMGR_ROOT}/lib:${LD_LIBRARY_PATH}"
hip_root=$(spack find -p hip | tail -n1 | grep hip | awk '{print $NF}' | sed 's/ //g')
message -c "hip_root: $hip_root"
export HIP_ROOT=$hip_root
CPATH+=":${HIP_ROOT}/include"
CPPFLAGS+=" -I ${HIP_ROOT} " 
CMAKE_PREFIX_PATH+=";${HIP_ROOT}"
CMAKE_EXE_LINKER_FLAGS+=" -L${HIP_ROOT}/lib "
LDFLAGS+=" -L${HIP_ROOT}/lib "
LD_LIBRARY_PATH="${HIP_ROOT}/lib:${LD_LIBRARY_PATH}"
hsa_rocr_root=$(spack find -p hsa-rocr-dev | tail -n1 | grep hsa-rocr | awk '{print $NF}' | sed 's/ //g') 
message -c "hsa_rocr_root: $hsa_rocr_root"
export HSA_ROCR_ROOT=$hsa_rocr_root
CPATH+=":${HSA_ROCR_ROOT}/include"
CPPFLAGS+=" -I ${HSA_ROCR_ROOT} " 
CMAKE_PREFIX_PATH+=";${HSA_ROCR_ROOT}"
CMAKE_EXE_LINKER_FLAGS+=" -L${HSA_ROCR_ROOT}/lib -lhsa-runtime64 "
LDFLAGS+=" -L${HSA_ROCR_ROOT}/lib -lhsa-runtime64 "
LD_LIBRARY_PATH="${HSA_ROCR_ROOT}/lib:${LD_LIBRARY_PATH}"
message -c "\$CPATH: $CPATH"
message -c "\$CPPFLAGS: $CPPFLAGS"
message -c "\$CMAKE_EXE_LINKER_FLAGS $CMAKE_EXE_LINKER_FLAGS"
export CPATH
export CPPFLAGS
export CMAKE_PREFIX_PATH
export CMAKE_EXE_LINKER_FLAGS
export LDFLAGS

# Additional libraries required for running samples
gcc_runtime_root=$(spack find -p gcc-runtime | tail -n1 | grep gcc-runtime | awk '{print $NF}' | sed 's/ //g')
LD_LIBRARY_PATH=${gcc_runtime_root}/lib:${LD_LIBRARY_PATH}

aqlprofile_root=$(spack find -p aqlprofile | tail -n1 | grep aqlprofile | awk '{print $NF}' | sed 's/ //g')
LD_LIBRARY_PATH=${aqlprofile_root}/lib:${LD_LIBRARY_PATH}

#cmake_exe_linker_flags="'-L${elf_root}/lib -ldw -L${hsakml_roct_root}/lib  -L${comgr_root}/lib -lamd_comgr -L${hip_root}/lib -L${hsa_rocr_root}/lib -lhsa-runtime64'"
mkdir -p ${SPACK_ROOT}/../rocprofiler-sdk
ROCPROFSDK_BUILD_ROOT=${SPACK_ROOT}/../rocprofiler-sdk-build
ROCPROFSDK_ROOT=${SPACK_ROOT}/../rocprofiler-sdk/${rocm_version}
LD_LIBRARY_PATH=${ROCPROFSDK_ROOT}/lib:${LD_LIBRARY_PATH}

[ -d rocprofiler-sdk-source ] && \
message -g "RUNNING: \
cmake \
  -B ${ROCPROFSDK_BUILD_ROOT} \
  -DROCPROFILER_BUILD_TESTS=ON \
  -DROCPROFILER_BUILD_SAMPLES=ON \
  -DCMAKE_INSTALL_PREFIX=${ROCPROFSDK_ROOT} \
  rocprofiler-sdk-source/ \
"
cmake \
  -B ${ROCPROFSDK_BUILD_ROOT} \
  -DROCPROFILER_BUILD_TESTS=ON \
  -DROCPROFILER_BUILD_SAMPLES=ON \
  -DCMAKE_INSTALL_PREFIX=${ROCPROFSDK_ROOT} \
  rocprofiler-sdk-source/
  
if [ $? -ne 0 ] ; then die "cmake configure failure" ; fi
message -g "RUNNING: \
cmake \
  --build ${ROCPROFSDK_BUILD_ROOT} \
  --target all \
  --parallel 8 \
"
cmake \
  --build ${ROCPROFSDK_BUILD_ROOT} \
  --target all \
  --parallel 8

if [ $? -ne 0 ] ; then die "cmake build failure" ; fi
message -g "RUNNING: \
cmake \
  --build ${ROCPROFSDK_BUILD_ROOT} \
  --target install \
"

cmake \
  --build ${ROCPROFSDK_BUILD_ROOT} \
  --target install

if [ $? -ne 0 ] ; then die "cmake install failure" ; fi

pushd ${ROCPROFSDK_ROOT}/share/rocprofiler-sdk/samples

export LD_LIBRARY_PATH
message -g "RUNNING: \
cmake \
  -B build-rocprofiler-sdk-samples \
  -DCMAKE_PREFIX_PATH=${ROCPROFSDK_ROOT} \
"
cmake \
  -B build-rocprofiler-sdk-samples \
  -DCMAKE_PREFIX_PATH=${ROCPROFSDK_ROOT} \

if [ $? -ne 0 ] ; then die "cmake samples configure error" ; fi

message -g "RUNNING: \
cmake \
  --build build-rocprofiler-sdk-samples \
  --target all \
  --parallel 8
"

cmake \
  --build build-rocprofiler-sdk-samples \
  --target all \
  --parallel 8

if [ $? -ne 0 ] ; then die "cmake samples build error" ; fi
echo $LD_LIBRARY_PATH | tr ":" "\n" | grep comgr
tmp_dir=$(mktemp -d /tmp/${USER}-ROCPROFSDK_SAMPLES_LOGS-XXXXX)
for directory in $(dirname $(find . -name "CTestTestfile.cmake")) ; do
  message -g "RUNNING ROCPROF-SDK-SAMPLES $directory VIA CTEST:"
  [[ -d $directory ]] && pushd $directory &>>/dev/null && ctest -V | tee ${tmp_dir}/${directory//*\/}_test.log | grep -i -e passed -e failed
  popd &>>/dev/null
done
pushd /tmp &>/dev/null
tar cvfJ ${tmp_dir//*\/}.tar.xz ${tmp_dir//*\/}
mv ${tmp_dir//*\/}.tar.xz $HOME/.
message -g "Test results are at ${HOME}/${tmp_dir//*\/}.tar.xz"
message -g "FINI: Check your results."
