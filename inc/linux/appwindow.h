#pragma once

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

struct Renderer;
struct Job;

struct AppWindow{
    //Window styles
    enum class WindowStyle {
        WS_EX_APPWINDOW  = (1l << 0),
        WS_EX_TOOLWINDOW = (1l << 1)
    };

    //Global status
    enum class APPLICATION_STATUS {
        EDIT_TEXT,
        MOVE_FONT,
        ROTATE_FONT,
        IN_DIALOG,
        OPEN_FILE,
        SAVE_FILE
    };

    AppWindow(unsigned int width, unsigned int height, const char* title, WindowStyle style);
    ~AppWindow();

    //Basic X11 vars
    Display *display;
    Window window;
    int screen;
    GLXContext glc;
    Colormap cmap;

    //Event handling
    XEvent xev;

    //Window attributes
    GLint att[15] = {
        GLX_RGBA,
        GLX_RED_SIZE,       8,
        GLX_GREEN_SIZE,     8,
        GLX_BLUE_SIZE,      8,
        GLX_ALPHA_SIZE,     8,
        GLX_DEPTH_SIZE,     24,
        GLX_STENCIL_SIZE,   0,          //no stencil
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo *vi;
    XSetWindowAttributes swa;
    XWindowAttributes gwa;

    //Renderer for this window
    Renderer* windowRenderer;
    //Job for this window
    Job* job;

    unsigned int width;
    unsigned int height;
    const char* title;
    bool isVisible = true;
    bool isToolBoxWindow = false;

    void show();
};
