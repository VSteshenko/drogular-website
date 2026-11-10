cmake_minimum_required(VERSION 3.24)

project(drogular_website VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Website is a separate consumer of Drogular; do not build the framework's
# examples, tests or tools as part of this application.
set(DROGULAR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(DROGULAR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DROGULAR_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    drogular
    GIT_REPOSITORY https://github.com/VSteshenko/drogular.git
    GIT_TAG v0.25.0
)
FetchContent_MakeAvailable(drogular)

configure_file(
    src/application/website_version.hpp.in
    "${PROJECT_BINARY_DIR}/generated/website_version.hpp"
    @ONLY
)

add_library(website_options STATIC
    src/application/website_options.cpp
)
target_include_directories(website_options PUBLIC src)
target_compile_features(website_options PUBLIC cxx_std_20)

# Both the executable and its tests use the very same application setup.
add_library(website_application STATIC
    src/application/website_application.cpp
    src/features/home/home_routes.cpp
)
target_include_directories(website_application PUBLIC src)
target_link_libraries(website_application PUBLIC website_options Drogular::drogular)

add_executable(drogular_website src/main.cpp)
target_link_libraries(drogular_website PRIVATE website_application)
target_include_directories(drogular_website PRIVATE "${PROJECT_BINARY_DIR}/generated")

option(DROGULAR_WEBSITE_BUILD_TESTS "Build website tests" ON)
if(DROGULAR_WEBSITE_BUILD_TESTS)
    include(CTest)
    enable_testing()

    # Match the GoogleTest release used by Drogular itself. Prefer a compatible
    # installed package; otherwise fetch it as a website-only test dependency.
    find_package(GTest 1.17.0 CONFIG QUIET)
    if(NOT TARGET GTest::gtest_main)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    find_package(Threads REQUIRED)

    add_executable(website_tests
        tests/application/website_options_tests.cpp
        tests/application/website_application_tests.cpp
        tests/http/website_http_tests.cpp
        tests/support/website_test_support.cpp
    )
    target_link_libraries(website_tests PRIVATE website_application GTest::gtest_main Threads::Threads)
    target_include_directories(website_tests PRIVATE tests)
    target_compile_definitions(website_tests PRIVATE
        WEBSITE_TEST_BINARY="$<TARGET_FILE:drogular_website>"
        WEBSITE_TEST_SOURCE_ROOT="${PROJECT_SOURCE_DIR}"
        WEBSITE_TEST_VERSION="${PROJECT_VERSION}"
    )
    add_dependencies(website_tests drogular_website)

    include(GoogleTest)
    gtest_discover_tests(website_tests DISCOVERY_TIMEOUT 30 PROPERTIES TIMEOUT 35)
endif()
