cmake_minimum_required(VERSION 3.28)

project(ShaderAlchemy LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---------- Dependencies ----------
# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLEW

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.3-docking
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)

    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )

    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

    target_link_libraries(imgui PUBLIC glfw)
endif()

# meshoptimizer
FetchContent_Declare(
    meshoptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
    GIT_TAG        v0.25
)
FetchContent_MakeAvailable(meshoptimizer)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# Assimp
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v6.0.2
)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

# ---------- Sources ----------
set(JIN_GL_SOURCES
    src/JinGL/GL.cpp
    src/JinGL/Platform.cpp
    src/JinGL/Debug.cpp
    src/JinGL/Buffer.cpp
    src/JinGL/Framebuffer.cpp
    src/JinGL/Shader.cpp
    src/JinGL/Texture2D.cpp
    src/JinGL/TextureLoader.cpp
    src/JinGL/VertexInput.cpp
)

set(TEXT_EDITOR_SOURCE
    src/ImGuiColorTextEdit/TextEditor.cpp
    src/ImGuiColorTextEdit/TextEditor.h
    src/ImGuiColorTextEdit/TextDiff.cpp
    src/ImGuiColorTextEdit/TextDiff.h
    src/ImGuiColorTextEdit/dtl.h
)

set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/EditorPanel.cpp
    src/FullScreenRenderPass.cpp
    src/Geometry.cpp
    src/ImGuiConsole.cpp
    src/Model.cpp
    src/ModelInputRenderPass.cpp
    src/RenderPass.cpp
    src/ShaderProgramSource.cpp
    src/stb_image.cpp
    src/Utils.cpp
    src/glad/gl.c
    src/glad/gl.h
    src/glad/wgl.h
)

# ---------- Executable ----------
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable(ShaderAlchemy ${SOURCES} ${JIN_GL_SOURCES} ${TEXT_EDITOR_SOURCE})

set_target_properties(ShaderAlchemy PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_include_directories(ShaderAlchemy PUBLIC src)

target_include_directories(
	ShaderAlchemy
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/src/ImGuiColorTextEdit
		${CMAKE_CURRENT_SOURCE_DIR}/src/JinGL
		${CMAKE_CURRENT_SOURCE_DIR}/src/Utils

		${imgui_SOURCE_DIR}
		${imgui_SOURCE_DIR}/backends
)
# ---------- Linking ----------
target_link_libraries(ShaderAlchemy
    PRIVATE
        glfw
        imgui
        meshoptimizer
        glm
        assimp
)



set(FFMPEG_URL "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip")
set(FFMPEG_ARCHIVE "${CMAKE_BINARY_DIR}/ffmpeg.zip")
set(FFMPEG_DIR "${CMAKE_BINARY_DIR}/ffmpeg")

if(NOT EXISTS ${FFMPEG_DIR})
    message(STATUS "Downloading FFmpeg from ${FFMPEG_URL}")
    file(DOWNLOAD ${FFMPEG_URL} ${FFMPEG_ARCHIVE} SHOW_PROGRESS)

    message(STATUS "Extracting FFmpeg...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${FFMPEG_ARCHIVE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # The extracted folder usually has a version in its name
    file(GLOB FFMPEG_EXTRACTED_DIR "${CMAKE_BINARY_DIR}/ffmpeg-*")
    file(RENAME "${FFMPEG_EXTRACTED_DIR}" "${FFMPEG_DIR}")

	file(REMOVE ${FFMPEG_ARCHIVE})
endif()

# Add to PATH or copy exe to your binary dir
add_custom_command(TARGET ShaderAlchemy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FFMPEG_DIR}/bin/ffmpeg.exe"
            $<TARGET_FILE_DIR:ShaderAlchemy>
)
