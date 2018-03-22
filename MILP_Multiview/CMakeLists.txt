cmake_minimum_required( VERSION 3.1.0 FATAL_ERROR )
project( Multiview )

set(CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package( Boost 1.40 COMPONENTS system program_options serialization filesystem iostreams REQUIRED )

find_package(OpenMP QUIET)
if(OPENMP_FOUND)
   MESSAGE( STATUS "OpenMP found" )
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_definitions(-DIL_STD)
find_package( CPLEX REQUIRED )

FILE(GLOB OptimalSrc src/*.cpp)

add_executable( optimal ${OptimalSrc})
target_include_directories( optimal PRIVATE inc ${Boost_INCLUDE_DIR} ${CPLEX_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries( optimal ${Boost_LIBRARIES} ${CPLEX_LIBRARIES} ${OpenCV_LIBRARIES})
