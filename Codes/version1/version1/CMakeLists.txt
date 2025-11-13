################################################################################
# Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
################################################################################

cmake_minimum_required ( VERSION 2.8.12 )
set ( CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "Verbose Output" FORCE )

## Linux Compiler options
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions" )

project ("LDMS - AMD GPUSampler")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set ( ROCM_PATH $ENV{ROCM_PATH} )
set ( ROCM_LIB_PATH ${ROCM_PATH}/lib )
set ( ROCP_INC_PATH ${ROCM_PATH}/include/rocprofiler )
set ( HSA_INC_PATH ${ROCM_PATH}/include/hsa )
set ( TOOL_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set ( OVIS_PATH $ENV{OVIS} )
set ( OVIS_LIB_PATH ${OVIS_PATH}/lib64 )
set ( OVIS_INC_PATH ${OVIS_PATH}/include )

## Find hsa-runtime library
find_package ( hsa-runtime64 CONFIG REQUIRED HINTS ${CMAKE_INSTALL_PREFIX} PATHS ${ROCM_PATH} )

## Find Rocprofiler library
find_library ( ROCPROFILER NAMES rocprofiler64 REQUIRED PATHS ${ROCM_LIB_PATH} )

## Util sources
file ( GLOB UTIL_SRC "${TOOL_DIR}/util/*.cpp" )

## Find Ovis
find_library ( OVISLIB NAMES ldms REQUIRED PATHS ${OVIS_LIB_PATH} )
find_library ( OVISSTREAMLIB NAMES ldmsd_stream REQUIRED PATHS ${OVIS_LIB_PATH} )

## Build tool injector
set ( TEST_LIB "rocp-tool" )
set ( TEST_LIB_SRC ${TOOL_DIR}/tool.cpp ${UTIL_SRC} )
add_library ( ${TEST_LIB} SHARED ${TEST_LIB_SRC} )
target_include_directories ( ${TEST_LIB} PRIVATE ${TOOL_DIR} ${ROCP_INC_PATH} ${HSA_INC_PATH} ${OVIS_INC_PATH} )
target_link_libraries ( ${TEST_LIB} ${ROCPROFILER} ${OVISLIB} ${OVISSTREAMLIB} hsa-runtime64::hsa-runtime64 Threads::Threads dl )
