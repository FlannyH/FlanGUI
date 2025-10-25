cmake_minimum_required(VERSION 3.16)
project(flangui LANGUAGES CXX)

# Options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# Source root
set(src_root "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(all_src
    "${src_root}/graphics/opengl/device_opengl.cpp"
    "${src_root}/graphics/opengl/device_opengl.hpp"
    "${src_root}/graphics/device.cpp"
    "${src_root}/graphics/device.hpp"
    "${src_root}/graphics/renderer.cpp"
    "${src_root}/graphics/renderer.hpp"
    "${src_root}/graphics/transform.cpp"
    "${src_root}/graphics/transform.hpp"
    "${src_root}/ui/panel.cpp"
    "${src_root}/ui/panel.hpp"
    "${src_root}/ui/panel_manager.cpp"
    "${src_root}/ui/panel_manager.hpp"
    "${src_root}/ui/components.hpp"
    "${src_root}/ui/scene.cpp"
    "${src_root}/ui/scene.hpp"
    "${src_root}/ui/value_system.hpp"
    "${src_root}/colors.hpp"
    "${src_root}/common.hpp"
    "${src_root}/input.hpp"
    "${src_root}/input.cpp"
    "${src_root}/log.cpp"
    "${src_root}/log.hpp"
)

# sanity check: only keep files that actually exist (avoids configure-time errors)
foreach(f IN LISTS all_src)
    if(NOT EXISTS "${f}")
        message(FATAL_ERROR "Missing source file: ${f}")
    endif()
endforeach()

# Create static library
add_library(flangui STATIC ${all_src})

# Position independent code for static libs (required on many linux toolchains)
set_target_properties(flangui PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Public include dirs (headers live under source/)
target_include_directories(flangui
    PUBLIC
        "${src_root}"
        "${src_root}/graphics"
        "${src_root}/ui"
)

# Compiler warnings and sane flags
if(MSVC)
    target_compile_options(flangui PRIVATE /W4 /permissive-)
else()
    target_compile_options(flangui PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
    # enable link-time-optimization if available
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-flto" COMPILER_SUPPORTS_FLO)
    if(COMPILER_SUPPORTS_FLO)
        target_compile_options(flangui PRIVATE -flto)
        set_target_properties(flangui PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# Platform-specific public defines (if you want to use them in code)
if(WIN32)
    target_compile_definitions(flangui PUBLIC PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(flangui PUBLIC PLATFORM_UNIX)
endif()

# Dependencies
add_subdirectory(external/glfw)
add_subdirectory(external/glbinding)
add_subdirectory(external/glm)
add_subdirectory(external/tomlplusplus)
target_include_directories(flangui PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/external/glm/
  ${CMAKE_CURRENT_SOURCE_DIR}/external/stb/
  ${CMAKE_CURRENT_SOURCE_DIR}/external/tomlplusplus/include/
)
target_link_libraries(flangui 
  glfw
  glbinding::glbinding
  glm::glm
  portaudio
  rtmidi
)