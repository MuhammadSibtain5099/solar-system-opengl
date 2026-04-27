cmake_minimum_required(VERSION 3.16)
project(SolarSystem)

set(CMAKE_CXX_STANDARD 17)

# ── Find OpenGL (always works on Windows) ────────────────────────────────────
find_package(OpenGL REQUIRED)

# ── GLFW3 ────────────────────────────────────────────────────────────────────
# If using vcpkg, this will be found automatically via the toolchain file.
# If not using vcpkg, set GLFW3_ROOT to your GLFW install directory, e.g.:
#   cmake .. -DGLFW3_ROOT="C:/libs/glfw"
find_package(glfw3 QUIET)

if (NOT glfw3_FOUND)
    if (NOT GLFW3_ROOT)
        message(FATAL_ERROR
            "glfw3 not found via CMake.\n"
            "Either:\n"
            "  (a) Use vcpkg:  cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake\n"
            "  (b) Set path:   cmake .. -DGLFW3_ROOT=C:/path/to/glfw\n"
        )
    endif()
    add_library(glfw STATIC IMPORTED)
    set_target_properties(glfw PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW3_ROOT}/include"
        IMPORTED_LOCATION             "${GLFW3_ROOT}/lib-vc2022/glfw3.lib"
    )
endif()

# ── GLEW ─────────────────────────────────────────────────────────────────────
find_package(GLEW QUIET)

if (NOT GLEW_FOUND)
    if (NOT GLEW_ROOT)
        message(FATAL_ERROR
            "GLEW not found via CMake.\n"
            "Either use vcpkg or set:  cmake .. -DGLEW_ROOT=C:/path/to/glew\n"
        )
    endif()
    add_library(GLEW::GLEW STATIC IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GLEW_ROOT}/include"
        IMPORTED_LOCATION             "${GLEW_ROOT}/lib/Release/x64/glew32s.lib"
    )
    add_compile_definitions(GLEW_STATIC)
endif()

# ── GLM (header-only) ─────────────────────────────────────────────────────────
find_package(glm QUIET)
if (NOT glm_FOUND AND GLM_ROOT)
    add_library(glm::glm INTERFACE IMPORTED)
    set_target_properties(glm::glm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GLM_ROOT}"
    )
endif()

# ── Executable ───────────────────────────────────────────────────────────────
add_executable(solar_system main.cpp)

target_include_directories(solar_system PRIVATE
    ${CMAKE_SOURCE_DIR}       # stb_image.h lives here
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(solar_system
    OpenGL::GL
    glfw
    GLEW::GLEW
)

if (TARGET glm::glm)
    target_link_libraries(solar_system glm::glm)
endif()

# On Windows, link additional system libs needed by GLFW
if (WIN32)
    target_link_libraries(solar_system gdi32 user32 shell32)
endif()

# ── Copy shaders & textures to build output directory ────────────────────────
add_custom_command(TARGET solar_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vertex.glsl
        ${CMAKE_SOURCE_DIR}/fragment.glsl
        $<TARGET_FILE_DIR:solar_system>
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:solar_system>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/textures
        $<TARGET_FILE_DIR:solar_system>/textures
    COMMENT "Copying shaders and textures..."
)
