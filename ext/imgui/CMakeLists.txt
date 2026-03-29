cmake_minimum_required(VERSION 3.5)

# Maybe stop from CMAKEing in the wrong place
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Source and build directories cannot be the same. Go use the /build directory.")
endif()


# # Append Wno-error to avoid compiling imgui with Werror; imgui seems to trigger lots of warnings
# if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")

#   # apparently, this stronger command is needed to accomplish the same in Clang
#   if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
#     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-everything")
#   endif()
# endif()

set(SRCS imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_demo.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp)

add_library(imgui ${SRCS})

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/")
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends")
target_include_directories(imgui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../glfw/include/")
target_include_directories(imgui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../glad/include/")

target_link_libraries(imgui PRIVATE glfw)

# On Windows/Linux, use the glad openGL loader
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(imgui PRIVATE glad)

set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE TRUE)