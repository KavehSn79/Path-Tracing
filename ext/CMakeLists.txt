# GLAD (OpenGL loader)
add_subdirectory(glad)
# ImGui (GUI framework)
add_subdirectory(imgui)
# STB Image (Image loader)
add_subdirectory(stb_image)

add_subdirectory(tinyobj)
add_subdirectory(json)

# CGTUB (rendering library)
set(CGTUB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CGTUB_USE_CUSTOM_DEPENDENCIES ON CACHE BOOL "" FORCE)
add_subdirectory(cgtub)

# GLFW (window and OpenGL context management)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
add_subdirectory(glm)