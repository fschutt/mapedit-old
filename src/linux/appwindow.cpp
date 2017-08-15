#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <X11/Xresource.h>
#include <X11/keysymdef.h>

//X11 defines
#define _NET_WM_STATE_REMOVE    0l
#define _NET_WM_STATE_ADD       1l
#define _NET_WM_STATE_TOGGLE    2l
#define GET_ATOM(X) data->X = XInternAtom(display, #X, False)

//Keyboard defines
#define MOD_SHIFT   (1l << 0)
#define MOD_CTRL    (1l << 1)
#define MOD_ALT     (1l << 2)

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <string>
#include <sstream>
#include <math.h>

#include "appwindow.h"
#include "print_log_console.h"
#include "errors.h"
#include "messages.h"
#include "job.h"
#include "renderer.h"

//Window class implementation for Linux
AppWindow::AppWindow(unsigned int initWidth, unsigned int initHeight, const char* initTitle, WindowStyle windowStyle)
{
    width = initWidth;
    height = initHeight;
    title = initTitle;

    if(setlocale(LC_ALL, "") == NULL){
        printErr("Locale not set.");
        return;
    }

    if(!XSupportsLocale()){
        printErr("Locale not supported by X Window System.");
        return;
    }

    if(XSetLocaleModifiers("@im=none") == NULL){
        printErr("Locale modifiers could not be set.");
    }

    //Create Window
    if ((display = XOpenDisplay(NULL)) == NULL) {
        printErr(ERR_X_SERVER_NO_CONNECT);
        return;
    }

    //Set screen
    screen = DefaultScreen(display);

    //Standard color mode (RGBA) & window type
    window = DefaultRootWindow(display);
    if ((vi = glXChooseVisual(display, screen, att)) == NULL) {
        printErr(ERR_X_MONITOR_MODE_NOT_SUPPORTED);
        return;
    }

    //Create drawing area in window
    if((cmap = XCreateColormap(display, window, vi->visual, AllocNone)) == BadMatch){
        printErr(ERR_X_MONITOR_MODE_NOT_SUPPORTED);
        return;
    }

    swa.colormap = cmap;

    //Only process CTRL, SHIFT & ALT & Mouse events
    swa.event_mask = ExposureMask | ControlMask | ShiftMask | Mod1Mask | KeyPressMask | \
    StructureNotifyMask | SubstructureNotifyMask | KeyReleaseMask;

    window = XCreateWindow(display, window, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

    //Hint window manager for toolbox windows
    switch(windowStyle){
        case WindowStyle::WS_EX_TOOLWINDOW: {
            struct MwmHints {
                unsigned long flags;
                unsigned long functions;
                unsigned long decorations;
                long input_mode;
                unsigned long status;
            };

            enum {
                MWM_HINTS_FUNCTIONS = (1L << 0),
                MWM_HINTS_DECORATIONS =  (1L << 1),

                MWM_FUNC_ALL = (1L << 0),
                MWM_FUNC_RESIZE = (1L << 1),
                MWM_FUNC_MOVE = (1L << 2),
                MWM_FUNC_MINIMIZE = (1L << 3),
                MWM_FUNC_MAXIMIZE = (1L << 4),
                MWM_FUNC_CLOSE = (1L << 5)
            };

            Atom mwmHintsProperty = XInternAtom(display, "_MOTIF_WM_HINTS", 0);
            struct MwmHints hints;
            hints.flags = MWM_HINTS_DECORATIONS;
            hints.decorations = 0;
            XChangeProperty(display, window, mwmHintsProperty, mwmHintsProperty, 32, PropModeReplace, (unsigned char *)&hints, 5);
        } break;
        case WindowStyle::WS_EX_APPWINDOW: {
            //TODO: doesn't work yet
            int count = 2;
            Atom atoms[16];
            Atom atomToChange = XInternAtom(display, "_NET_WM_STATE", True);

            Atom _NET_WM_STATE_MAXIMIZED_VERT = _NET_WM_STATE_ADD;
            Atom _NET_WM_STATE_MAXIMIZED_HORZ = _NET_WM_STATE_ADD;

            atoms[0] = _NET_WM_STATE_MAXIMIZED_VERT;
            atoms[1] = _NET_WM_STATE_MAXIMIZED_HORZ;

            int err = XChangeProperty(display, window, atomToChange, XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, count);
            printf("%d\n", err);
        } break;
    }

    //Map window onto screen
    if(XMapWindow(display, window) == BadWindow){
        printErr(ERR_X_SERVER_BAD_WINDOW);
    }

    if(XStoreName(display, window, title) == BadAlloc){
        printErr(ERR_X_SERVER_BAD_ALLOC);
    }else if(XStoreName(display, window, title) == BadWindow){
        printErr(ERR_X_SERVER_BAD_WINDOW);
    }

    //Setup OpenGL context for window
    glc = glXCreateContext(display, vi, NULL, GL_TRUE);
    glXMakeCurrent(display, window, glc);
}

//Window should automatically draw, without using show
//Problems when using two GL contexts
void AppWindow::show()
{
    //Setup event handling
    Bool owner_events = False;
    KeySym key;
    char text[255];
    XIM im; //input method
    XIC ic; //input char
    XIMStyles *styles;
    XIMStyle xim_requested_style;
    char *failed_arg;

    im = XOpenIM(display, NULL, NULL, NULL);
    if (im == NULL) {
        printErr("Could not open input method.");
        return;
    }

    failed_arg = XGetIMValues(im, XNQueryInputStyle, &styles, NULL);

    if (failed_arg != NULL) {
      printErr("XIM Can't get styles");
      return;
    }

    //Print all styles
    for (int i = 0; i < styles->count_styles; i++) {
        printf("style %d\n", (int)styles->supported_styles[i]);
    }

    //Create input character (may be multi-byte)
    ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, NULL);
    if (ic == NULL) {
        printErr("Could not open input char.");
        return;
    }

    XSetICFocus(ic);

    char rawKey[20];
    std::string buf;                    //current stream of characters
    Status status = 0;                  //possible error
    int mouseButtonDragX = 0;
    int mouseButtonDragY = 0;
    bool mouseButtonDown = false;

    //TODO: Shortcut map handler
    //TODO: UTF8 doesn't work yet
    //TODO: Maximize window doesn't work yet
    bool moveMapMode;
    bool zoomMapMode;
    bool editTextMode   = true;
    bool drawBezierMode;

    std::map<long, bool*> shortcutMap = {
        {27l, &moveMapMode}                   //ESCAPE
    };


    //Application status - if switched, buf is cleared
    //Actions are restricted depending on mode
    APPLICATION_STATUS globalStatus = APPLICATION_STATUS::EDIT_TEXT;

    //JobManager opens and loads projects, etc.
    while (isVisible) {
        XNextEvent(display, &xev);
        switch(xev.type){
            case Expose:{
                if(xev.xexpose.count == 1){
                    //JobManager::startNextJob();
                }
            } break;
            case KeymapNotify:{
                XRefreshKeyboardMapping(&xev.xmapping);
            } break;
            ///-----------
            case KeyPress:{
                    //Also generate a click event because some shortcuts require
                    //having a position to relate to a drag event
                    mouseButtonDown = true;
                    mouseButtonDragX = xev.xkey.x;
                    mouseButtonDragY = xev.xkey.y;

                    Xutf8LookupString(ic, (XKeyPressedEvent*)&xev, rawKey, 20, &key, &status);
                    if (status==XBufferOverflow){ break; }

                    for(auto shortcut : shortcutMap){
                        if((long)*rawKey == shortcut.first){
                            printf("Pressed hotkey - %l\n", xev.xkey.serial);
                            *shortcut.second = true;
                            break;
                        }
                    }

                    //If not a shortcut
                    buf.append(rawKey);
                    printf("%s\n", buf.c_str());
                    printf("pressed KEY: %d\n", (long)*rawKey);
            } break;
            case KeyRelease:{
                ///-----------
                ///KEYBOARD INPUT


            } break;
            ///-----------
            ///WINDOW MOVED
            case ConfigureNotify: {
                printf("ConfigureNotify\n");
                if (width != xev.xconfigure.width || height != xev.xconfigure.height){
                    width = xev.xconfigure.width;
                    height = xev.xconfigure.height;
                    //X11 update window width & height
                    printf("Size changed to: %d by %d\n", width, height);
                }
            } break;
            ///-----------
            case ButtonPress:{
                mouseButtonDown = true;
                //In case the user starts to drag
                mouseButtonDragX = xev.xbutton.x;
                mouseButtonDragY = xev.xbutton.y;
            } break;
            ///-----------
            case ButtonRelease:{
                if(mouseButtonDown){
                    mouseButtonDown = false;
                    int distanceX = xev.xbutton.x - mouseButtonDragX;
                    int distanceY = xev.xbutton.y - mouseButtonDragY;
                    if(sqrt(pow(distanceX, 2) * pow(distanceY, 2)) < 10){
                        if(xev.xbutton.button != 1){
                            ///MOUSE BUTTON LEFT
                            printf("Left MB pressed: %d\t%d\n", xev.xbutton.x, xev.xbutton.y);
                        }else{
                            ///MOUSE BUTTON RIGHT
                            printf("Right MB pressed: %d\t%d\n", xev.xbutton.x, xev.xbutton.y);
                        }
                    }else{
                        ///DRAG
                        printf("Drag! Distance: %d\t%d\n", distanceX, distanceY);
                    }
                }
            }
        }//end switch

        glXSwapBuffers(display, window);

    } // end(main window loop)

    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, glc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

AppWindow::~AppWindow(){
    delete windowRenderer;
    if(job != nullptr){
        delete job;
    }
}
