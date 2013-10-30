
set(glfwdiag_SOURCES diag.c win32.c)
set(glfwdiag_HEADERS resource.h diag.h)
set(glfwdiag_RESOURCES main.rc)

add_executable(glfwdiag WIN32 ${glfwdiag_SOURCES}
                              ${glfwdiag_HEADERS}
                              ${glfwdiag_RESOURCES})

target_link_libraries(glfwdiag glfw ${GLFW_LIBRARIES})

