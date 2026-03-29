function(add_exercise NAME TAG)
    string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" EXERCISE_STANDALONE)
    if (EXERCISE_STANDALONE)
        include(cmake/common.cmake)
        add_subdirectory(ext)

        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)

        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
    endif()
    
    set(EXECUTABLE_NAME "${NAME}")
    if(NOT ${TAG} STREQUAL "")
        set(EXECUTABLE_NAME "${NAME}-${TAG}")
    endif()

    file(GLOB EXERCISE_SOURCE_FILES src/*.*)

    add_executable(${EXECUTABLE_NAME} ${EXERCISE_SOURCE_FILES})
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE glm::glm glfw glad imgui json tinyobj cgtub)

    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE -DASSETS_DIRECTORY="${CMAKE_CURRENT_LIST_DIR}/assets"
                                                          -DSOURCE_DIRECTORY="${CMAKE_CURRENT_LIST_DIR}/src")
endfunction()