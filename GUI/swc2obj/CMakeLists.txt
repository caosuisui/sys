cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 14)

include(cmake/lidanBase.cmake)
include(cmake/LidanBuild.cmake)

SET(BLOCKSIZE 256)
SET(RADIUS 1)
SET(ROOT_DIR ${CMAKE_SOURCE_DIR})
set(CONDIF_DIR "${CMAKE_SOURCE_DIR}/")


include(ProcessorCount)
ProcessorCount(processor_N)
if(NOT processor_N EQUAL 0)
    set(CTEST_BUILD_FLAGS -j${processor_N})
    set(ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${N})
endif()

configure_file (
        "${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config/config.h"
)

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#set(VTK_DIR "G:/VTK/lib/cmake/vtk-8.2")
find_package(VTK REQUIRED)

#set(TBB_DIR "F:/oneapi-tbb-2021.5.0-win/oneapi-tbb-2021.5.0/lib/cmake/tbb")
#find_package(TBB REQUIRED)

ADD_SUBSRC(datadir data)
ADD_SUBSRC(swc2voldir swc2vol/)
ADD_SUBSRC(utildir util)
ADD_SUBSRC(vol2objdir vol2obj/)
ADD_SUBSRC(mergeobjdir ObjMerge)

add_subdirectory(external/glm)
add_subdirectory(external/cxxopts)

add_executable(SWC2OBJ ${datadir} ${swc2voldir} ${utildir} ${vol2objdir} ${mergeobjdir} main.cpp)

add_library(SWC2VOLLIB ${datadir} ${swc2voldir} ${utildir} ${vol2objdir} ${mergeobjdir})

target_include_directories(SWC2VOLLIB
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/data/"
        "${CMAKE_CURRENT_SOURCE_DIR}/swc2vol/"
        "${CMAKE_CURRENT_SOURCE_DIR}/vol2obj/"
        "${CMAKE_CURRENT_SOURCE_DIR}/util/"

        "${CMAKE_CURRENT_BINARY_DIR}/config/"
        "${CMAKE_CURRENT_SOURCE_DIR}/ObjMerge/"


        "${CMAKE_CURRENT_SOURCE_DIR}/external/glm/"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/cxxopts/include"
        )

target_link_libraries(SWC2VOLLIB
        PUBLIC
        ${VTK_LIBRARIES}
        )


target_include_directories(SWC2OBJ
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/data/"
        "${CMAKE_CURRENT_SOURCE_DIR}/swc2vol/"
        "${CMAKE_CURRENT_SOURCE_DIR}/vol2obj/"
        "${CMAKE_CURRENT_SOURCE_DIR}/util/"

        "${CMAKE_CURRENT_BINARY_DIR}/config/"
        "${CMAKE_CURRENT_SOURCE_DIR}/ObjMerge/"


        "${CMAKE_CURRENT_SOURCE_DIR}/external/glm/"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/cxxopts/include"
        )



target_link_libraries(SWC2OBJ
        PUBLIC
        ${VTK_LIBRARIES}
        )


