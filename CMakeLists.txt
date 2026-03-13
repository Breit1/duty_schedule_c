cmake_minimum_required(VERSION 3.15)
project(DutyScheduleApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

set(SOURCES
    src/main.cpp
    src/duty_manager.cpp
    src/duty_manager.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    httplib::httplib
    nlohmann_json::nlohmann_json
)

file(COPY ${CMAKE_SOURCE_DIR}/web DESTINATION ${CMAKE_BINARY_DIR})


