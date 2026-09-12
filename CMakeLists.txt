cmake_minimum_required(VERSION 3.20)

project(apigen
    VERSION 0.1.0
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies -------

include(FetchContent)

# ----------------------------------------
# Dependencies
# ----------------------------------------

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

# ----------------------------------------
# Application
# ----------------------------------------

add_executable(apigen)

target_sources(apigen
    PRIVATE
        src/main.cpp
        src/core/converter.cpp
        src/parser/har_parser.cpp
        src/generator/typescript_generator.cpp
)

target_include_directories(apigen
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(apigen
    PRIVATE
        nlohmann_json::nlohmann_json
)

# ----------------------------------------
# Tests
# ----------------------------------------

enable_testing()

add_executable(apigen_tests
    tests/har_parser_test.cpp
    src/parser/har_parser.cpp
)

target_include_directories(apigen_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(apigen_tests
    PRIVATE
        nlohmann_json::nlohmann_json
        GTest::gtest_main
)

target_compile_definitions(apigen_tests
    PRIVATE
        TEST_DATA_DIR="${PROJECT_SOURCE_DIR}/tests/fixtures"
)

include(GoogleTest)

gtest_discover_tests(apigen_tests)
