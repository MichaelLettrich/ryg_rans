CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0 FATAL_ERROR)

project(ALICERANS)


# find scripts:
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

message("${CMAKE_MODULE_PATH}")
add_subdirectory(libcommon)
add_subdirectory(librans)
add_subdirectory(examples)