#
# Tool library used by rocprofiler
#

rocprofiler_activate_clang_tidy()

set(TOOL_HEADERS config.hpp execution_profile.hpp helper.hpp stream_stack.hpp)

set(TOOL_SOURCES config.cpp main.c tool.cpp stream_stack.cpp)

set ( OVIS_PATH $ENV{OVIS} )
set ( OVIS_INC_PATH ${OVIS_PATH}/include )
set ( OVIS_LIB_PATH ${OVIS_PATH}/lib )
find_library ( OVISLIB NAMES ldms required PATHS ${OVIS_LIB_PATH} )
find_library ( OVISSTREAMLIB NAMES ldmsd_stream REQUIRED PATHS ${OVIS_LIB_PATH} )

add_library(rocprofiler-sdk-tool SHARED)
target_sources(rocprofiler-sdk-tool PRIVATE ${TOOL_SOURCES} ${TOOL_HEADERS})

target_include_directories (rocprofiler-sdk-tool 
	PRIVATE
		${OVIS_INC_PATH}
)

target_link_libraries(
    rocprofiler-sdk-tool
    PRIVATE rocprofiler-sdk::rocprofiler-sdk-shared-library
            rocprofiler-sdk::rocprofiler-sdk-headers
            rocprofiler-sdk::rocprofiler-sdk-build-flags
            rocprofiler-sdk::rocprofiler-sdk-memcheck
            rocprofiler-sdk::rocprofiler-sdk-common-library
            rocprofiler-sdk::rocprofiler-sdk-output-library
            rocprofiler-sdk::rocprofiler-sdk-cereal
            rocprofiler-sdk::rocprofiler-sdk-perfetto
            rocprofiler-sdk::rocprofiler-sdk-otf2
            rocprofiler-sdk::rocprofiler-sdk-dw
            rocprofiler-sdk::rocprofiler-sdk-amd-comgr
            rocprofiler-sdk::rocprofiler-sdk-att-parser
	    ${OVISLIB}
	    ${OVISSTREAMLIB} )

set_target_properties(
    rocprofiler-sdk-tool
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY
               ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/rocprofiler-sdk
               SOVERSION ${PROJECT_VERSION_MAJOR}
               VERSION ${PROJECT_VERSION}
               BUILD_RPATH "\$ORIGIN:\$ORIGIN/.."
               INSTALL_RPATH "\$ORIGIN:\$ORIGIN/..")

install(
    TARGETS rocprofiler-sdk-tool
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/rocprofiler-sdk
    COMPONENT tools
    EXPORT rocprofiler-sdk-tool-targets)

target_compile_definitions(rocprofiler-sdk-tool PRIVATE "-D_GL_HAS_ATTRIBUTE(x)=0")

add_subdirectory(kokkosp)
