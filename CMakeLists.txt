cmake_minimum_required( VERSION 2.8.12 )

project( puny_coder_prj )

include( ExternalProject )

find_package( Boost 1.58.0 COMPONENTS system filesystem regex unit_test_framework iostreams REQUIRED )
find_package( Threads )

add_definitions( -DBOOST_TEST_DYN_LINK ) 

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL 'MSVC' )
	add_compile_options( -D_WIN32_WINNT=0x0601 ) 
else( )
	if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" )
		add_compile_options(-std=c++14 -ferror-limit=1 -Weverything -Wno-c++98-compat -Wfatal-errors -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion)		
	elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
		#add_compile_options(-std=c++14 -Wall -Werror -fmax-errors=1 -Wno-deprecated-declarations)
		add_compile_options(-std=c++14 -Wall -Wno-deprecated-declarations)
	endif()
endif()

externalproject_add( 
	header_libraries_prj
	GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/header_libraries"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

externalproject_add(
	char_range_prj
	GIT_REPOSITORY "https://github.com/beached/char_range.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/char_range"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

externalproject_add(
	parse_json_prj
	GIT_REPOSITORY "https://github.com/beached/parse_json.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/parse_json"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)


set( HEADER_FOLDER "include" )
include_directories( ${HEADER_FOLDER} )

set( HEADER_FILES
	${HEADER_FOLDER}/puny_coder.h
)

set( SOURCE_FILES
	puny_coder.cpp
 )

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )
link_directories( "${CMAKE_BINARY_DIR}/install/lib" )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

add_library( puny_coder ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( puny_coder header_libraries_prj char_range_prj )
target_link_libraries( puny_coder char_range ${Boost_LIBRARIES} )

install( TARGETS puny_coder DESTINATION lib )
install( DIRECTORY ${HEADER_FOLDER}/ DESTINATION include/daw/puny_coder )

add_executable( puny_coder_test_bin puny_coder_test.cpp ${HEADER_FILES} )
add_dependencies( puny_coder_test_bin parse_json_prj )
target_link_libraries( puny_coder_test_bin puny_coder parse_json char_range ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
add_test( puny_coder_test, puny_coder_test_bin )

