set(CGTUB_INCLUDE_DIR "../../include/cgtub")

add_library(cgtub STATIC ${CGTUB_INCLUDE_DIR}/attribute_buffer.hpp attribute_buffer.cpp
                         ${CGTUB_INCLUDE_DIR}/camera.hpp camera.cpp
                         ${CGTUB_INCLUDE_DIR}/camera_orthographic.hpp camera_orthographic.cpp
                         ${CGTUB_INCLUDE_DIR}/camera_perspective.hpp camera_perspective.cpp
			             ${CGTUB_INCLUDE_DIR}/camera_controller.hpp camera_controller.cpp
			             ${CGTUB_INCLUDE_DIR}/camera_controller_turntable.hpp camera_controller_turntable.cpp
                         ${CGTUB_INCLUDE_DIR}/canvas.hpp canvas.cpp
                         ${CGTUB_INCLUDE_DIR}/event_dispatcher.hpp event_dispatcher.cpp
                         ${CGTUB_INCLUDE_DIR}/fwd.hpp
			             ${CGTUB_INCLUDE_DIR}/geometry.hpp geometry.cpp
                         ${CGTUB_INCLUDE_DIR}/gl_wrap.hpp gl_wrap.cpp
                         ${CGTUB_INCLUDE_DIR}/image.hpp image.cpp
                         ${CGTUB_INCLUDE_DIR}/image_renderer.hpp image_renderer.cpp
                         ${CGTUB_INCLUDE_DIR}/line_renderer.hpp line_renderer.cpp 
                         ${CGTUB_INCLUDE_DIR}/log.hpp log.cpp 
                         ${CGTUB_INCLUDE_DIR}/mesh_renderer.hpp mesh_renderer.cpp mesh_renderer_shaders.hpp
                         ${CGTUB_INCLUDE_DIR}/primitives.hpp
                         ${CGTUB_INCLUDE_DIR}/point_renderer.hpp point_renderer.cpp 
                         ${CGTUB_INCLUDE_DIR}/render_pipeline.hpp render_pipeline.cpp
                         ${CGTUB_INCLUDE_DIR}/simple_renderer.hpp simple_renderer.cpp
                         ${CGTUB_INCLUDE_DIR}/texture_buffer.hpp texture_buffer.cpp
                         ${CGTUB_INCLUDE_DIR}/ndc_renderer.hpp ndc_renderer.cpp
)

target_link_libraries(cgtub PUBLIC glad glfw imgui glm::glm stb_image)
target_include_directories(cgtub PUBLIC "${CGTUB_INCLUDE_DIR}/..")

if (CGTUB_LEGACY_OUTPUTS)
    target_compile_definitions(cgtub PRIVATE -DCGTUB_LEGACY_OUTPUTS=1)
endif()

#if (MSVC)
#    # Statically link the MSVC runtime (should mitigate installation issues; maybe)
#    set_target_properties(cgtub PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#endif()

# TODO: Versioning
set_target_properties(cgtub PROPERTIES VERSION "0.0.1" SOVERSION "1")

# Copy the shared libraries the target depends on into the output folder
# see: https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:TARGET_RUNTIME_DLLS
# add_custom_command(TARGET exe POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:exe> $<TARGET_RUNTIME_DLLS:exe>
#   COMMAND_EXPAND_LISTS
# )

# Make target installable
install(TARGETS cgtub
    EXPORT cgtubTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)