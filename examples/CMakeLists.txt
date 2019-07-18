CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0 FATAL_ERROR)

project(examples LANGUAGES CXX)

# find requirements
find_package(RapidJSON 1.0 REQUIRED MODULE)
find_package(docopt CONFIG REQUIRED )

# add a 32 bit and 64bit executable of ransbenchmark.
foreach(exec rans32Benchmark.exe;rans64Benchmark.exe)
  add_executable(${exec} ransBenchmark.cpp)
  # the 32 bit version required the rans32 define
  if("${exec}" STREQUAL "rans32Benchmark.exe")
	target_compile_definitions(${exec} PRIVATE -Drans32)
	message("32bit eneabled")
  endif()
	target_link_libraries(${exec}
		PRIVATE
			RapidJSON::RapidJSON
			docopt_s
			rans
			common
	)
endforeach()

include(GNUInstallDirs)
install (TARGETS rans32Benchmark.exe rans64Benchmark.exe 
	RUNTIME DESTINATION bin
	)