# CMake minimum version.
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set project and languages.
project(lisp_interpreter/ LANGUAGES CXX)

# Set standard
set (CMAKE_CXX_STANDARD 20)

# Add compiler flags
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_compile_options(-fsanitize=undefined)
add_link_options(-fsanitize=undefined)

add_compile_options(-fsanitize=leak)
add_link_options(-fsanitize=leak)

add_compile_options(-Wall -Wextra -Wno-error -Wshadow -Wpedantic)

# Add executable program.
add_executable(lisp app/main.cpp lib/ast.cpp lib/parse.cpp lib/env.cpp lib/utility.cpp)
target_include_directories(lisp PUBLIC include/)

# Install main program.
install(TARGETS lisp DESTINATION bin)

# Install the demo script.
install(PROGRAMS demo DESTINATION bin)

# Install example programs.
install(DIRECTORY examples/ DESTINATION bin/examples)
