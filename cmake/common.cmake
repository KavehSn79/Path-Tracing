set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_DEBUG_POSTFIX d)

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
    message(STATUS "Disabling -Wunsafe-buffer-usage and -Wno-poison-system-directories warnings for Clang >= 16")
    add_compile_options(-Wno-unsafe-buffer-usage -Wno-poison-system-directories)
  endif()
endif()