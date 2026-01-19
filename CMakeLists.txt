# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.21)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("OUCTool")

# ---------------- C++ 标准 ----------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- 源码路径 ----------------
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# ---------------- 依赖 ----------------
include(cmake/imgui.cmake)
include(cmake/cpr.cmake)
include(cmake/json.cmake)

# ---------------- 可执行文件 ----------------
file(GLOB_RECURSE SRC_MAIN
    "${SRC_DIR}/*.hpp"
    "${SRC_DIR}/*.h"
    "${SRC_DIR}/*.cpp"
)

add_executable(OUCTool ${SRC_MAIN})

source_group(TREE ${SRC_DIR} PREFIX "Source Files" FILES ${SRC_MAIN})

# ---------------- 头文件 ----------------
target_include_directories(OUCTool PRIVATE
    ${SRC_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
    ${json_SOURCE_DIR}/single_include
    ${cpr_SOURCE_DIR}/include
)

# ---------------- 链接库 ----------------
target_link_libraries(OUCTool PRIVATE
    imgui
    cpr
    d3d11
    dxgi
    dwmapi
)
# ---------------- MSVC 特定 ----------------
if(MSVC)
    add_compile_options(/bigobj)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()