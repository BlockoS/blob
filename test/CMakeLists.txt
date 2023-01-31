file(WRITE ${CMAKE_BINARY_DIR}/stb.c "#define STB_IMAGE_IMPLEMENTATION\n#include \"stb_image.h\"\n#define STB_IMAGE_WRITE_IMPLEMENTATION
#include \"stb_image_write.h\"\n")
add_library(stb STATIC ${CMAKE_BINARY_DIR}/stb.c)
target_include_directories(stb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(label label.c)
target_link_libraries(label blob stb m)
target_compile_options(label PRIVATE -Wall -Wshadow -Wextra)
