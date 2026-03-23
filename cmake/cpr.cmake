include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CPR_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)
sset(CPR_USE_LIB_PSL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.14.2
)
FetchContent_MakeAvailable(cpr)
