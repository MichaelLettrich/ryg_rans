CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0 FATAL_ERROR)

project(libcommon VERSION 0.1.0.0 LANGUAGES CXX)

add_library(common STATIC )
target_sources(common PRIVATE 
	src/executionTimer.cpp 
	src/helper.cpp
	)
target_include_directories(common PUBLIC include)
target_compile_features(common PUBLIC cxx_std_17)

find_package(RapidJSON 1.0 REQUIRED MODULE)
target_link_libraries(common
	PUBLIC
		RapidJSON::RapidJSON
		)
		
include(GNUInstallDirs)
install (TARGETS common
	EXPORT commonTargets
	LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
	)
	
install (
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}
	DESTINATION include
	)