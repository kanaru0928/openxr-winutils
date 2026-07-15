cmake_minimum_required(VERSION 3.21)
project(OpenXRWinUtils LANGUAGES CXX)

if(NOT WIN32)
    message(WARNING "OpenXR WinUtils targets Windows x64 only; configuring for reference/CI purposes.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

# openvr ships prebuilt headers/lib/dll and also a legacy CMakeLists.txt
# (cmake_minimum_required 2.8) that rebuilds the library from source and is
# incompatible with modern CMake policies. Point SOURCE_SUBDIR at a
# subdirectory with no CMakeLists.txt so FetchContent only fetches the
# sources without add_subdirectory()-ing the legacy build.
FetchContent_Declare(
    openvr
    GIT_REPOSITORY https://github.com/ValveSoftware/openvr.git
    GIT_TAG v2.5.1
    SOURCE_SUBDIR headers
)
FetchContent_MakeAvailable(openvr)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.9
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

add_executable(OpenXRWinUtils WIN32
    src/main.cpp
    src/overlay_app.cpp
    src/gui.cpp
    src/restart_advsettings.cpp
    src/d3d11_renderer.cpp
)

target_include_directories(OpenXRWinUtils PRIVATE
    ${openvr_SOURCE_DIR}/headers
)

target_link_libraries(OpenXRWinUtils PRIVATE
    imgui
    d3d11
    dxgi
    d3dcompiler
)

if(WIN32)
    set(OPENVR_LIB_DIR ${openvr_SOURCE_DIR}/lib/win64)
    set(OPENVR_BIN_DIR ${openvr_SOURCE_DIR}/bin/win64)
    target_link_libraries(OpenXRWinUtils PRIVATE ${OPENVR_LIB_DIR}/openvr_api.lib)

    add_custom_command(TARGET OpenXRWinUtils POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${OPENVR_BIN_DIR}/openvr_api.dll
            $<TARGET_FILE_DIR:OpenXRWinUtils>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/manifest.vrmanifest
            $<TARGET_FILE_DIR:OpenXRWinUtils>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/assets/icon.png
            $<TARGET_FILE_DIR:OpenXRWinUtils>
    )
endif()
