cmake_minimum_required(VERSION 3.5.0)
project(main VERSION 0.1.0)

# 使用 C++ 11 标准
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR})


# 添加头文件
set(GLAD_H ${PROJECT_SOURCE_DIR}/dependencies/include)
# set(GLFW_H /usr/local/include)
# set(GLFW_H ${PROJECT_SOURCE_DIR}/dependencies/include/GLFW)
set(LEARNOPENGL_INCLUDE ${PROJECT_SOURCE_DIR}/learnopengl)  # 新增这一行，指定learnopengl目录的路径
include_directories(${GLAD_H} ${GLFW_H} ${LEARNOPENGL_INCLUDE})

# 添加目标链接
# set(GLFW_LINK /usr/local/lib/libglfw.3.dylib)
set(GLFW_LINK ${PROJECT_SOURCE_DIR}/dependencies/library/libglfw.3.3.dylib)
set(ASSIMP_LINK ${PROJECT_SOURCE_DIR}/dependencies/library/libassimp.dylib)
set(LIB_LINK ${PROJECT_SOURCE_DIR}/dependencies/library/libzlibstatic.a)

link_libraries(${GLFW_LINK})
link_libraries(${ASSIMP_LINK})
link_libraries(${LIB_LINK})


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# 执行编译命令
# set(SOURCES glad.c main.cpp)
set(SOURCES main.cpp glad.c)
add_executable(main ${SOURCES})

# 链接系统的 OpenGL 框架
if (APPLE)
    target_link_libraries(main "-framework OpenGL")
endif()

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
