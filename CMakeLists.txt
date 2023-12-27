cmake_minimum_required(VERSION 3.5.0)
project(main VERSION 0.1.0)

# 使用 C++ 11 标准
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR})

#add_subdirectory(dependencies/include EXCLUDE_FROM_ALL)

# 添加头文件
set(GLAD_H ${PROJECT_SOURCE_DIR}/dependencies/include)
set(LEARNOPENGL_INCLUDE ${PROJECT_SOURCE_DIR}/learnopengl)  # 新增这一行，指定learnopengl目录的路径
#set(NOISE_H /usr)
set(MY_INCLUDE
        entity/tiny_obj_loader.h
        entity/MY_Model.h
        entity/Entity.h
        entity/Loader.h
        entity/Terrain.h
        entity/constants.h
        entity/Player.h
        entity/GameTime.h
        entity/Camera.h
        entity/InputState.h

        renders/EntityRenderer.h
        renders/RenderManager.h
        renders/SkyboxRenderer.h
        renders/TerrainRenderer.h
        renders/FrameBuffer.h
        renders/ShadowMap.h

        shaders/ShaderProgram.h
        shaders/TerrainShader.h
        shaders/EntityShader.h
        shaders/SkyboxShader.h

        Window.h)

set(MY_SRC
        entity/MY_Model.cpp
        entity/Entity.cpp
        entity/Loader.cpp
        entity/tiny_obj_loader.cpp
        entity/Terrain.cpp
        entity/Player.cpp
        entity/GameTime.cpp
        entity/Camera.cpp

        renders/EntityRenderer.cpp
        renders/RenderManager.cpp
        renders/SkyboxRenderer.cpp
        renders/TerrainRenderer.cpp
        renders/FrameBuffer.cpp
        renders/ShadowMap.cpp

        shaders/ShaderProgram.cpp
        shaders/TerrainShader.cpp
        shaders/EntityShader.cpp
        shaders/SkyboxShader.cpp

        Window.cpp)

include_directories(${GLAD_H} ${GLFW_H} ${LEARNOPENGL_INCLUDE} ${MY_INCLUDE})


# 添加目标链接
# set(GLFW_LINK /usr/local/lib/libglfw.3.dylib)
set(GLFW_LINK ${PROJECT_SOURCE_DIR}/dependencies/library/libglfw.3.3.dylib)
set(ASSIMP_LINK ${PROJECT_SOURCE_DIR}/dependencies/library/libassimp.dylib)
set(LIB_LINK ${PROJECT_SOURCE_DIR}/dependencies/library/libzlibstatic.a)
set(FMT_LINK ${PROJECT_SOURCE_DIR}/dependencies/include/fmt/build/libfmt.a)
set(NOISE_LINK ${PROJECT_SOURCE_DIR}/dependencies/library/libnoise.dylib)
set(NOISEUTIL_LINK ${PROJECT_SOURCE_DIR}/dependencies/library/libnoiseutils.dylib)

link_libraries(${GLFW_LINK})
link_libraries(${ASSIMP_LINK})
link_libraries(${LIB_LINK})
link_libraries(${FMT_LINK})
link_libraries(${NOISE_LINK})
link_libraries(${NOISEUTIL_LINK})


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# 执行编译命令
# set(SOURCES glad.c main.cpp)
set(SOURCES main.cpp glad.c ${MY_SRC} particles/particle.h particles/particle.cpp shaders/particleShader.h shaders/particleShader.cpp renders/particleRenderer.h renders/particleRenderer.cpp renders/ParticleManager.h renders/ParticleManager.cpp particles/particleSystem.h particles/ParticleSystem.cpp shaders/AABBShader.h shaders/AABBShader.cpp)
add_executable(main ${SOURCES})
#target_link_libraries(main PRIVATE fmt::fmt ${CMAKE_DL_LIBS})
target_include_directories(main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
# 链接系统的 OpenGL 框架
if (APPLE)
    target_link_libraries(main "-framework OpenGL")
endif()

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/image
        ${PROJECT_BINARY_DIR}/image
        COMMENT "Copy resources to build tree")

