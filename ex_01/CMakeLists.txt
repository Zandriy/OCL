# cmake -G "Visual Studio 12 2013 Win64" ../src

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(PRG ex_01)
project(${PRG} CXX C)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif (NOT MSVC)

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/../ThirdParty)

file(GLOB PRG_SRC *.cpp)
file(GLOB PRG_HDR *.h)

add_executable(${PRG} ${PRG_SRC} ${PRG_HDR})
target_link_libraries( ${PRG} ${OpenCL_LIBRARIES} )