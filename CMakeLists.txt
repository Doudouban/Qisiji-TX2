cmake_minimum_required(VERSION 2.8)
project(box_runtime_vertion)
# 添加c++ 11标准支持
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread" )
# 设置编译模式
set( CMAKE_BUILD_TYPE "Debug" )
#set( CMAKE_BUILD_TYPE "Release" )
# 变量的定义
set(TARGET_LIB tycam)
#set(INCLUDE_PATH ./include)

#添加common里面的头文件
include_directories(include)
#添加源文件
AUX_SOURCE_DIRECTORY(src DIR_SRCS)

#设置环境变量
SET(TEST_MATH ${DIR_SRCS})

# ========================================
# === Qt
# ========================================
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)
#set(CMAKE_PREFIX_PATH "/usr/bin/cmake")
set(CMAKE_PREFIX_PATH "/home/doudouban/Qt5.12.0/5.12.0/gcc_64/lib/cmake")
#set(CMAKE_PREFIX_PATH "/home/elsa/Qt/Qt5.12.0/5.12.0/gcc_64/lib/cmake")


find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)
#find_package(Qt5 COMPONENTS Core Gui Qml Quick REQUIRED)


set(UI_FILES)
set(RC_FILES)
set(QRC_FILES)

set(QT_LIB Qt5::Widgets)


# ========================================
# === OpenCV
# ========================================
find_package( OpenCV REQUIRED )
if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV library not found")
else()
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()
# ========================================
#添加pthread库
# ========================================
find_package(Threads REQUIRED)

#添加可执行文件
add_executable(box_runtime_vertion ${TEST_MATH} ${RC_FILES} ${UI_FILES} ${QRC_FILES})
#链接库,为target添加需要链接的共享库
target_link_libraries(box_runtime_vertion  ${TARGET_LIB} ${OpenCV_LIBS} ${QT_LIB} ${Threads_LIB})
