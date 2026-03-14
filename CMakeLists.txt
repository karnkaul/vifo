cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

include(version.cmake)

project(${package_name} VERSION ${build_version})

option(VIFO_BUILD_CLI "Build vifo CLI" ${PROJECT_IS_TOP_LEVEL})

add_subdirectory(ext)
add_subdirectory(lib)

if(VIFO_BUILD_CLI)
  add_subdirectory(cli)
endif()
