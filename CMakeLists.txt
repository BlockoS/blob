cmake_minimum_required (VERSION 3.1)

project(blob)

set(BLOB_MAJOR_VERSION 0)
set(BLOB_MINOR_VERSION 0)
set(BLOB_PATCH_VERSION 1)
set(BLOB_VERSION "${BLOB_MAJOR_VERSION}.${BLOB_MINOR_VERSION}.${BLOB_PATCH_VERSION}")

find_package(Doxygen)

set(PROJECT_NAME blob)

file(WRITE ${CMAKE_BINARY_DIR}/blob.c "#define BLOB_IMPLEMENTATION\n#include \"${CMAKE_SOURCE_DIR}/blob.h\"")
add_library(blob STATIC ${CMAKE_BINARY_DIR}/blob.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wshadow -Wextra -std=gnu11")

set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in)
set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY IMMEDIATE)

add_custom_target(doc
         COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
         SOURCES ${DOXYFILE}
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMENT "Generating API documentation with Doxygen"
         VERBATIM )
