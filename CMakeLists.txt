cmake_minimum_required (VERSION 3.20)

project(blob 
    VERSION 0.1.0
    DESCRIPTION "8-neighbour connected components labelling and contours extractor"
    HOMEPAGE_URL "https://codeberg.org/mooz/blob"
    LANGUAGES C
)

option(BUILD_TEST "Build test program" OFF)
option(BUILD_DOC  "Build documentation" OFF)

file(WRITE ${PROJECT_BINARY_DIR}/blob.c "#define BLOB_IMPLEMENTATION\n#include <blob.h>")
add_library(blob STATIC ${PROJECT_BINARY_DIR}/blob.c blob.h)
target_include_directories(blob PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> )
target_compile_options(blob PRIVATE $<IF:$<C_COMPILER_ID:MSVC>,/W4,-Wall -Wshadow -Wextra -Werror -Wconversion -Wno-sign-conversion>)

if(BUILD_TEST)
    # We are being lazy here. We don't build the test program on msvc because of getopt.
    if(MSVC)
        message(NOTICE "Test program is not available under Microsoft Visual C compilers")
    else()
        add_subdirectory(test)
    endif()
endif()

if(BUILD_DOC)
    # Doc generation
    find_package(Doxygen)

    set(DOXYFILE_IN ${PROJECT_SOURCE_DIR}/doxyfile.in)
    set(DOXYFILE ${PROJECT_BINARY_DIR}/doxyfile)

    configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY IMMEDIATE)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
        SOURCES ${DOXYFILE}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM 
    )
endif()
