cmake_minimum_required (VERSION 3.20)

project(blob 
    VERSION 0.0.1
    DESCRIPTION "8-neighbour connected components labelling and contours extractor"
    HOMEPAGE_URL "https://github.com/BlockoS/blob"
    LANGUAGES C
)

file(WRITE ${CMAKE_BINARY_DIR}/blob.c "#define BLOB_IMPLEMENTATION\n#include \"${CMAKE_CURRENT_SOURCE_DIR}/blob.h\"")
add_library(blob STATIC ${CMAKE_BINARY_DIR}/blob.c blob.h)
target_include_directories(blob PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> )
target_compile_options(blob PRIVATE $<IF:$<C_COMPILER_ID:MSVC>,/W4,-Wall -Wshadow -Wextra>)

if(NOT MSVC)                    # We are being lazy here. We don't build the test program on msvc because of getopt.
    add_subdirectory(test)
endif()

# Doc generation
find_package(Doxygen)

set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in)
set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY IMMEDIATE)

add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    SOURCES ${DOXYFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM 
)
