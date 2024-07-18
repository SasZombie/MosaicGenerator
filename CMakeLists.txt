cmake_minimum_required(VERSION 3.10)

project(Mosaic)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat-nonliteral -Wcast-align -Wpointer-arith -Wmissing-declarations -Winline -Wundef -Wcast-qual -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal -pedantic -ansi -fsanitize=address -fsanitize=leak")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

add_executable(main main.cpp stb_image.h stb_image_write.h stb_image_resize.h)
# add_executable(coloring coloring.cpp stb_image.h stb_image_write.h stb_image_resize.h)
# add_executable(uitest uitest.cpp UI.cpp)


target_link_libraries(main raylib GL m pthread dl rt X11)
# target_link_libraries(uitest raylib GL m pthread dl rt X11)