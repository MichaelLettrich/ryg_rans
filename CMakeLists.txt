CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0 FATAL_ERROR)

project(ALICERANS)

# find scripts:
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

add_compile_options("-Wall" "-Wextra" "-Werror" "-O3")

add_subdirectory(libcommon)
add_subdirectory(librans)
add_subdirectory(examples)
