CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0 FATAL_ERROR)

project(librans VERSION 0.1.0.0 LANGUAGES CXX)

add_library(rans STATIC )
target_sources(rans PRIVATE src/SymbolStatistics.cpp)
target_include_directories(rans PUBLIC include)
target_compile_features(rans PUBLIC cxx_std_17)

find_package(RapidJSON 1.0 REQUIRED MODULE)
target_link_libraries( rans
	PUBLIC
		RapidJSON::RapidJSON
		)
		
include(GNUInstallDirs)
install (TARGETS rans
	EXPORT ransTargets
	LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
	)

install (
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}
	DESTINATION include
	)
