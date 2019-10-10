CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0 FATAL_ERROR)

project(examples LANGUAGES CXX)

# find requirements
find_package(RapidJSON 1.0 REQUIRED MODULE)
find_package(docopt CONFIG REQUIRED )

set(tgts "")

# add a 32 bit and 64bit executable of ransbenchmark.
foreach(arch 32;64)
  foreach(bits 8;16;32)
	set(exec "rans${arch}Benchmark${bits}.exe")
	add_executable(${exec} ransBenchmark.cpp)
	list(APPEND tgts ${exec})
	target_compile_definitions(${exec} PRIVATE -DSOURCE_T=uint${bits}_t)
	# the 32 bit version required the rans32 define
	if("${arch}" STREQUAL "32")
		target_compile_definitions(${exec} PRIVATE -Drans32)
	endif()
		target_link_libraries(${exec}
			PRIVATE
				RapidJSON::RapidJSON
				docopt_s
				rans
				common
		)
  endforeach()
endforeach()

include(GNUInstallDirs)
install (TARGETS ${tgts} 
	RUNTIME DESTINATION bin
	)