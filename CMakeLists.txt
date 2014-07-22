cmake_minimum_required( VERSION 2.8.8 )

project( PhysFS-BFS )

if( CMAKE_COMPILER_IS_GNUCXX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
endif( CMAKE_COMPILER_IS_GNUCXX )

find_package( PhysFS REQUIRED )

include_directories( ${PHYSFS_INCLUDE_DIR} "src" "include" )

add_library( physfs-bfs SHARED
	src/bfsarchive.cpp src/bfsarchive.hpp
	src/bfsarchiver.cpp include/bfsarchiver.h
	src/bfsfile.cpp src/bfsfile.hpp
	src/bfsfilecompressed.cpp src/bfsfilecompressed.hpp
	src/bfsformat.hpp
	src/bitstream.cpp src/bitstream.hpp
	src/huffmann.cpp src/huffmann.hpp
	src/physfs_miniz.hpp
	src/stringpool.cpp src/stringpool.hpp
	src/zipstream.cpp src/zipstream.hpp
	)
target_link_libraries( physfs-bfs ${PHYSFS_LIBRARY} )

add_executable( physfs-bfs-test
	src/main.cpp
	include/bfsarchiver.h
	)
target_link_libraries( physfs-bfs-test physfs-bfs ${PHYSFS_LIBRARY} )

install( FILES include/bfsarchiver.h DESTINATION include )
install( TARGETS physfs-bfs RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib )
