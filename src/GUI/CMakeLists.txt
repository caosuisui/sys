
set(CMAKE_PREFIX_PATH G:\\Qt\\6.1.0\\msvc2019_64\\lib\\cmake)
#set(CMAKE_PREFIX_PATH C:\\Users\\csh\\opencv\\build\\x64\\vc15\\lib)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
#find_package(OpenMP REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(
        GLOB_RECURSE
        SYSGUI_SRC
        "*.cpp"
        "*.c"
        "*.hpp"
        "*.h"
)

add_executable(SYSGUI
        ${SYSGUI_SRC}
        )

set(FFmpeg_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/ffmpeg/include)
set(
        FFmpeg_LIBS
        ${PROJECT_SOURCE_DIR}/deps/ffmpeg/lib/avcodec.lib
        ${PROJECT_SOURCE_DIR}/deps/ffmpeg/lib/avutil.lib

)
include(${PROJECT_SOURCE_DIR}/deps/VolumeCompression.cmake)

target_link_libraries(
        SYSGUI
        PRIVATE
        SWC2VOLLIB
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        #VS_CORE
#        ${FFmpeg_LIBS}
        voxel_uncompress
)

target_include_directories(SYSGUI
        PRIVATE
        ../swc2obj
        ./src
        ${FFmpeg_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/deps
        )
