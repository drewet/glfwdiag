
#include <GLFW/glfw3.h>

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "diag.h"

static char* analysis = NULL;

static void append_to_analysis(const char* format, ...)
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

char* analyze(void)
{
    int i, monitorCount;
    GLFWmonitor** monitors;

    if (!glfwInit())
        return NULL;

    append_to_analysis("GLFWDIAG compiled on " __DATE__ "\n");
    append_to_analysis("GLFW %s\n", glfwGetVersionString());

    monitors = glfwGetMonitors(&monitorCount);
    for (i = 0;  i < monitorCount;  i++)
    {
        int j, modeCount;
        const GLFWvidmode* modes;

        append_to_analysis("Monitor %i (%s)\n", i, glfwGetMonitorName(monitors[i]));

        modes = glfwGetVideoModes(monitors[i], &modeCount);
        for (j = 0;  j < modeCount;  j++)
        {
            append_to_analysis("%i: %i x %i, %i BPP (%i %i %i), %i Hz\n",
                               j,
                               modes[j].width, modes[j].height,
                               modes[j].redBits + modes[j].greenBits + modes[j].blueBits,
                               modes[j].redBits, modes[j].greenBits, modes[j].blueBits,
                               modes[j].refreshRate);
        }
    }

    for (i = GLFW_JOYSTICK_1;  i < GLFW_JOYSTICK_LAST;  i++)
    {
        if (glfwJoystickPresent(i))
        {
            int axisCount, buttonCount;

            glfwGetJoystickAxes(i, &axisCount);
            glfwGetJoystickButtons(i, &buttonCount);

            append_to_analysis("Joystick %i (%s): %i axes, %i buttons\n",
                               i,
                               glfwGetJoystickName(i),
                               axisCount, buttonCount);
        }
        else
            append_to_analysis("Joystick %i: not present\n", i);
    }

    glfwTerminate();
    return analysis;
}

