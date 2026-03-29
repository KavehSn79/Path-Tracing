cmake_minimum_required(VERSION 3.14)

project(cgtub LANGUAGES CXX)

string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" CGTUB_STANDALONE)

option(CGTUB_BUILD_EXAMPLES "Build example applications" ${CGTUB_STANDALONE})
option(CGTUB_USE_CUSTOM_DEPENDENCIES "External dependencies are supplied by the including project" OFF)
option(CGTUB_LEGACY_OUTPUTS "Enable the beloved debug messages" OFF)

# Configure general CMake variables
set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable parallel builds for Visual Studio
if (MSVC)
	add_compile_options(/MP)
endif()

# Add central for windows
if (WIN32)
	add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

# Disable certain warnings on (Apple)Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(STATUS "Disabling -Wc++98-compat warnings for Clang")
	add_compile_options(-Wno-c++98-compat -Wno-c++98-compat-pedantic)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16)
    message(STATUS "Disabling -Wunsafe-buffer-usage warnings for Clang >= 16")
    add_compile_options(-Wno-unsafe-buffer-usage)
  endif()
endif()

#######################
# External libraries
#######################
if (NOT CGTUB_USE_CUSTOM_DEPENDENCIES)
  add_subdirectory(ext)
endif()

#######################
# Our libraries and applications
#######################
add_subdirectory(src/cgtub)

if (CGTUB_BUILD_EXAMPLES)
  add_subdirectory(examples/hello_world)
endif()