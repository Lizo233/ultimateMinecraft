#需求的最低cmake程序版本
cmake_minimum_required(VERSION 3.12)

#本工程的名字
project(Ultimate_Minecraft)

#本工程支持的C++版本
set(CMAKE_CXX_STANDARD 17)

#将/include加入头文件目录
include_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#链接/libs下的.lib文件
link_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

#将include其下所有.c/.cpp文件编译
file(GLOB_RECURSE CPPS ./*.c *.cpp)
add_library(cpps ${CPPS})

#将素材文件夹复制到执行目录
file(GLOB ASSETS "./assets" )
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

#用utf-8编译
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")

#已在CMakeSettings.json将当前项目设置为Debug模式

#生成exe
add_executable(my_Minecraft "main.cpp")

target_link_libraries(my_Minecraft glfw3.lib fmtd.lib zlibstatic.lib cpps)

#跨平台兼容（使用tinyfiledialogs）
if(UNIX AND NOT APPLE)
    target_link_libraries(my_Minecraft X11)
elseif(WIN32)
    target_link_libraries(my_Minecraft comdlg32 ole32 uuid)
endif()
