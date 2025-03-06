# CMake Build Configuration for root of project
cmake_minimum_required(VERSION 3.18)
project(my_hello_world VERSION 1.0 DESCRIPTION "Starter project" LANGUAGES C)

# Compiler options (inherited by sub-folders)
set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Werror -Wpedantic -Wextra -g)
add_compile_options(-fdiagnostics-color)

# Enable address sanitizer
# (Comment this out to make your code faster)
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

# Enable PThread library for linking
add_compile_options(-pthread)
add_link_options(-pthread)

# Define _POSIX_C_SOURCE to enable POSIX-specific functionality (e.g., nanosleep)
add_compile_definitions(_POSIX_C_SOURCE=200809L)

# What folders to build
add_subdirectory(lgpio)
add_subdirectory(lcd)
add_subdirectory(hal)  
add_subdirectory(app)

