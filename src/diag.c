
#include <GLFW/glfw3.h>

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "diag.h"

static char* analysis = NULL;

static void append(const char* format, ...)
{
    va_list vl;
    char buffer[65536];

    va_start(vl, format);
    if (vsnprintf(buffer, sizeof(buffer), format, vl) < 0)
    buffer[sizeof(buffer) - 1] = '\0';
    va_end(vl);

    if (analysis)
    {
        const size_t length = strlen(buffer) + strlen(analysis) + 1;
        analysis = realloc(analysis, length);
        strcat(analysis, buffer);
    }
    else
        analysis = strdup(buffer);
}

static void append_separator(void)
{
    append("\r\n");
}

static void error_callback(int error, const char* description)
{
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

char* analyze(void)
{
    int i, monitorCount;
    GLFWmonitor** monitors;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return NULL;

    append("GLFWDIAG compiled on " __DATE__ "\r\n");
    append("GLFW %s\r\n", glfwGetVersionString());

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
        append("%s monitor %i (%s)\r\n",
               primary ? "Primary" : "Secondary",
               i,
               glfwGetMonitorName(monitors[i]));

        append("Current mode: %s\r\n", format_video_mode(mode));
        append("Virtual position: %i %i\r\n", xpos, ypos);

        dpi = (int) ((float) mode->width * 25.4f / (float) widthMM);
        append("Physical size: %i x %i mm (%i dpi)\r\n", widthMM, heightMM, dpi);

        append("Modes:\r\n");
        modes = glfwGetVideoModes(monitors[i], &modeCount);
        for (j = 0;  j < modeCount;  j++)
            append("%4i: %s\r\n", j, format_video_mode(modes + j));
    }

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

    glfwTerminate();
    return analysis;
}

