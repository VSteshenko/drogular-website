cmake_minimum_required(VERSION 3.24)

project(drogular_website VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(DROGULAR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(DROGULAR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DROGULAR_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    drogular
    GIT_REPOSITORY https://github.com/VSteshenko/drogular.git
    GIT_TAG v0.25.0
)

FetchContent_MakeAvailable(drogular)

add_executable(drogular_website
    src/main.cpp
    src/application/website_application.cpp
    src/features/home/home_routes.cpp
)

target_link_libraries(drogular_website
    PRIVATE
        Drogular::drogular
)

# Website-local headers (application/..., features/...).
target_include_directories(drogular_website
    PRIVATE
        src
)