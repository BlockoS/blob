cmake_minimum_required (VERSION 3.1)

project(label)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wshadow -Wextra -std=gnu11")

include_directories(.)
add_executable(label label.c blob.c stb_image.c stb_image_write.c)
target_link_libraries(label m)
