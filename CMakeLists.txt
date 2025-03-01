cmake_minimum_required(VERSION 3.28)

set(VCPKG_DIR "C:/vcpkg")

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")


set(CMAKE_CXX_STANDARD 20)

project(ShaderAlchemy)

set(JIN_GL_SOURCES
    src/JinGL/Debug.cpp
    src/JinGL/Buffer.cpp
    src/JinGL/Framebuffer.cpp
    src/JinGL/Shader.cpp
    src/JinGL/Texture2D.cpp
    src/JinGL/TextureLoader.cpp
    src/JinGL/VertexInput.cpp
)

set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/EditorPanel.cpp
    src/FullScreenRenderPass.cpp
    src/Geometry.cpp
    src/ImGuiConsole.cpp
    src/TextEditor.cpp
    src/Model.cpp
    src/ModelInputRenderPass.cpp
    src/RenderPass.cpp
    src/ShaderProgramSource.cpp
    src/stb_image.cpp
    src/Utils.cpp
)


add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable(ShaderAlchemy ${SOURCES} ${JIN_GL_SOURCES})
target_include_directories(ShaderAlchemy PUBLIC src)

find_package(GLEW REQUIRED)
target_link_libraries(ShaderAlchemy PRIVATE GLEW::GLEW)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(ShaderAlchemy PRIVATE glfw)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(ShaderAlchemy PRIVATE imgui::imgui)

find_package(meshoptimizer CONFIG REQUIRED)
target_link_libraries(ShaderAlchemy PRIVATE meshoptimizer::meshoptimizer)

find_package(glm CONFIG REQUIRED)
target_link_libraries(ShaderAlchemy PRIVATE glm::glm)


find_package(assimp CONFIG REQUIRED)
target_link_libraries(ShaderAlchemy PRIVATE assimp::assimp)