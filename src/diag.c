
#include <GLFW/glfw3.h>

#include <GL/glext.h>

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "diag.h"

#define API_OPENGL          "gl"
#define API_OPENGL_ES       "es"

#define PROFILE_NAME_CORE   "core"
#define PROFILE_NAME_COMPAT "compat"

#define STRATEGY_NAME_NONE  "none"
#define STRATEGY_NAME_LOSE  "lose"

static char* report = NULL;

static void append(const char* format, ...)
{
    va_list vl;
    char buffer[65536];

    va_start(vl, format);
    if (vsnprintf(buffer, sizeof(buffer), format, vl) < 0)
        buffer[sizeof(buffer) - 1] = '\0';
    va_end(vl);

    if (report)
    {
        const size_t length = strlen(buffer) + strlen(report) + 1;
        report = realloc(report, length);
        strcat(report, buffer);
    }
    else
        report = strdup(buffer);
}

static void append_separator(void)
{
    append("\r\n");
}

static void error_callback(int error, const char* description)
{
    append_separator();
    append("%s\r\n", description);
}

static const char* format_video_mode(const GLFWvidmode* mode)
{
    static char buffer[512];

    sprintf(buffer,
            "%i x %i x %i (%i %i %i) %i Hz",
            mode->width, mode->height,
            mode->redBits + mode->greenBits + mode->blueBits,
            mode->redBits, mode->greenBits, mode->blueBits,
            mode->refreshRate);

    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

static const char* get_client_api_name(int api)
{
    if (api == GLFW_OPENGL_API)
        return "OpenGL";
    else if (api == GLFW_OPENGL_ES_API)
        return "OpenGL ES";

    return "Unknown API";
}

static const char* get_profile_name_gl(GLint mask)
{
    if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
        return PROFILE_NAME_COMPAT;
    if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
        return PROFILE_NAME_CORE;

    return "unknown";
}

static const char* get_profile_name_glfw(int profile)
{
    if (profile == GLFW_OPENGL_COMPAT_PROFILE)
        return PROFILE_NAME_COMPAT;
    if (profile == GLFW_OPENGL_CORE_PROFILE)
        return PROFILE_NAME_CORE;

    return "unknown";
}

static const char* get_strategy_name_gl(GLint strategy)
{
    if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
        return STRATEGY_NAME_LOSE;
    if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
        return STRATEGY_NAME_NONE;

    return "unknown";
}

static const char* get_strategy_name_glfw(int strategy)
{
    if (strategy == GLFW_LOSE_CONTEXT_ON_RESET)
        return STRATEGY_NAME_LOSE;
    if (strategy == GLFW_NO_RESET_NOTIFICATION)
        return STRATEGY_NAME_NONE;

    return "unknown";
}

int report_init(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return 0;

    append("GLFWDIAG compiled on " __DATE__ "\r\n");
    append("GLFW %s\r\n", glfwGetVersionString());

    return 1;
}

void report_terminate(void)
{
    glfwTerminate();
}

void report_monitors(void)
{
    int i, monitorCount;
    GLFWmonitor** monitors;

    monitors = glfwGetMonitors(&monitorCount);
    for (i = 0;  i < monitorCount;  i++)
    {
        int j, xpos, ypos, widthMM, heightMM, dpi, modeCount;
        const GLFWvidmode* modes;
        const int primary = (glfwGetPrimaryMonitor() == monitors[i]);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

        glfwGetMonitorPos(monitors[i], &xpos, &ypos);
        glfwGetMonitorPhysicalSize(monitors[i], &widthMM, &heightMM);

        append_separator();
        append("Monitor %i (%s) %s\r\n",
               i,
               glfwGetMonitorName(monitors[i]),
               primary ? "primary" : "secondary");

        append("Current mode: %s\r\n", format_video_mode(mode));
        append("Virtual position: %i %i\r\n", xpos, ypos);

        dpi = (int) ((float) mode->width * 25.4f / (float) widthMM);
        append("Physical size: %i x %i mm (%i dpi)\r\n", widthMM, heightMM, dpi);

        append("Modes:\r\n");
        modes = glfwGetVideoModes(monitors[i], &modeCount);
        for (j = 0;  j < modeCount;  j++)
            append("%4i: %s\r\n", j, format_video_mode(modes + j));
    }
}

void report_joysticks(void)
{
    int i;

    append_separator();

    for (i = GLFW_JOYSTICK_1;  i < GLFW_JOYSTICK_LAST;  i++)
    {
        if (glfwJoystickPresent(i))
        {
            int axisCount, buttonCount;

            glfwGetJoystickAxes(i, &axisCount);
            glfwGetJoystickButtons(i, &buttonCount);

            append("Joystick %i (%s): %i axes, %i buttons\r\n",
                   i,
                   glfwGetJoystickName(i),
                   axisCount, buttonCount);
        }
        else
            append("Joystick %i: not present\r\n", i);
    }
}

void report_context(void)
{
    GLFWwindow* window = glfwGetCurrentContext();
    const int api = glfwGetWindowAttrib(window, GLFW_CLIENT_API);
    const int major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    const int minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    const int rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);

    append_separator();
    append("%s context version string: \"%s\"\r\n",
           get_client_api_name(api),
           glGetString(GL_VERSION));
    append("%s context version parsed by GLFW: %u.%u.%u\r\n",
           get_client_api_name(api),
           major, minor, rev);

    if (api == GLFW_OPENGL_API)
    {
        if (major >= 3)
        {
            GLint flags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            append("%s context flags (0x%08x):", get_client_api_name(api), flags);

            if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
                append(" forward-compatible");
            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
                append(" debug");
            if (flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB)
                append(" robustness");
            append("\r\n");

            append("%s context flags parsed by GLFW:", get_client_api_name(api));

            if (glfwGetWindowAttrib(window, GLFW_OPENGL_FORWARD_COMPAT))
                printf(" forward-compatible");
            if (glfwGetWindowAttrib(window, GLFW_OPENGL_DEBUG_CONTEXT))
                printf(" debug");
            if (glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS) != GLFW_NO_ROBUSTNESS)
                printf(" robustness");
            append("\r\n");
        }

        if (major > 3 || (major == 3 && minor >= 2))
        {
            GLint mask;
            int profile = glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE);

            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
            append("%s profile mask (0x%08x): %s\r\n",
                   get_client_api_name(api),
                   mask,
                   get_profile_name_gl(mask));

            append("%s profile mask parsed by GLFW: %s\r\n",
                   get_client_api_name(api),
                   get_profile_name_glfw(profile));
        }

        if (glfwExtensionSupported("GL_ARB_robustness"))
        {
            int robustness;
            GLint strategy;
            glGetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);

            append("%s robustness strategy (0x%08x): %s\r\n",
                   get_client_api_name(api),
                   strategy,
                   get_strategy_name_gl(strategy));

            robustness = glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS);

            append("%s robustness strategy parsed by GLFW: %s\r\n",
                   get_client_api_name(api),
                   get_strategy_name_glfw(robustness));
        }
    }

    append("%s context renderer string: \"%s\"\r\n",
           get_client_api_name(api),
           glGetString(GL_RENDERER));
    append("%s context vendor string: \"%s\"\r\n",
           get_client_api_name(api),
           glGetString(GL_VENDOR));

    if (major > 1)
    {
        append("%s context shading language version: \"%s\"\r\n",
               get_client_api_name(api),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
    }
}

void report_extensions(void)
{
    int i;
    GLint count;
    const GLubyte* extensions;
    GLFWwindow* window = glfwGetCurrentContext();

    append_separator();
    append("%s context supported extensions:\r\n",
           get_client_api_name(glfwGetWindowAttrib(window, GLFW_CLIENT_API)));

    if (glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR) > 2)
    {
        PFNGLGETSTRINGIPROC glGetStringi =
            (PFNGLGETSTRINGIPROC) glfwGetProcAddress("glGetStringi");
        if (!glGetStringi)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glGetIntegerv(GL_NUM_EXTENSIONS, &count);

        for (i = 0;  i < count;  i++)
            append("%s\r\n", glGetStringi(GL_EXTENSIONS, i));
    }
    else
    {
        extensions = glGetString(GL_EXTENSIONS);
        while (*extensions != '\0')
        {
            if (*extensions == ' ')
                append("\r\n");
            else
                append("%c", *extensions);

            extensions++;
        }
    }
}

char* get_report(void)
{
    return report;
}

int test_default_window(void)
{
    GLFWwindow* window;
    double base;

    append_separator();
    append("Creating a default window\r\n");

    glfwDefaultWindowHints();

    base = glfwGetTime();

    window = glfwCreateWindow(640, 480, "Window Title", NULL, NULL);
    if (!window)
        return 0;

    append("Creating the window took %0.3f seconds\r\n", glfwGetTime() - base);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    report_context();
    report_extensions();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwDestroyWindow(window);
    return 1;
}

