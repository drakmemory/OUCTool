include(FetchContent)

set(JSON_MultipleHeaders OFF)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_PROGRESS TRUE
)
message("json")
FetchContent_MakeAvailable(json)