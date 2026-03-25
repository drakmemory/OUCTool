include(FetchContent)

FetchContent_Declare(
    wil
    GIT_REPOSITORY https://github.com/microsoft/wil.git
    GIT_TAG master
)

# 👇 关键：只下载，不 add_subdirectory
FetchContent_GetProperties(wil)
if(NOT wil_POPULATED)
    FetchContent_Populate(wil)
endif()

# 只导出 include 路径
set(WIL_INCLUDE_DIR "${wil_SOURCE_DIR}/include")