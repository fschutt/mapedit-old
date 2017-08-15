#include "server.h"
#include "print_log_console.h"
#include "string.h"
#include <stdio.h>
#include <map>
#include <regex>

#include <GL/glx.h>
#include <GL/gl.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <signal.h>

#define GLX_CONTEXT_MAJOR_VERSION_ARB		0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB		0x2092

#include "map_defs.h"
/*------------------------------------- HELPER FUNCTIONS ----------------------------------------------*/

// Helper to check for extension string presence.  Adapted from:
// https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
bool isExtensionSupported(const char *extList, const char *extension)
{
  const char *start;
  const char *where, *terminator;

  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  if (where || *extension == '\0'){
    return false;
  }

  //It takes a bit of care to be fool-proof about parsing the OpenGL extensions string. Don't be fooled by sub-strings, etc.
  for (start=extList;;) {
    where = strstr(start, extension);

    if (!where){
        break;
    }

    terminator = where + strlen(extension);

    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return true;

    start = terminator;
  }

  return false;
}
/*------------------------------------- HELPER FUNCTIONS ----------------------------------------------*/

/*---------------------------------------- NETWORKING -------------------------------------------------*/
Server& Server::getInstance()
{
    //Singleton instance
    //Guaranteed to be lazy evaluated and destroyed correctly
    //Instanitated on first use
    static Server instance;
    return instance;
}

bool Server::start(unsigned int portNumber){

    //TCP Socket
    socketFileDescriptor= socket(AF_INET, SOCK_STREAM, 0);
    if(socketFileDescriptor < 0){
        printErr("Error opening socket.");
        return false;
    }

    memset((char*)&serverIPAndPort, 0, sizeof(serverIPAndPort));

    if(portNumber != 0){
        portNr = portNumber;
    }else{
        //TODO: load from config file
        //default portnumber
        //previos know port: 3493 (nut), next known port 3632 (distcc)
        portNr = 3500;
    }

    //If port is used, use next port
    int retries = 100;
    while(errno == EADDRINUSE && retries > 0){
        serverIPAndPort.sin_family = AF_INET6;
        serverIPAndPort.sin_addr.s_addr = INADDR_ANY;
        serverIPAndPort.sin_port = htons(portNr);
        portNr++;
        retries--;
    }

    if(retries == 0){
        printErr("Could not find a free port after 100 retries. \n \
                  Reconfigure mapedit.conf and start again.");
        return false;
    }

    if(bind(socketFileDescriptor,
            (struct sockaddr*)&serverIPAndPort,
            sizeof(serverIPAndPort)) < 0){
        printErr("Error on binding port. \n \
                  Another application is probably using the same port (3500 on default settings).");
        return false;
    }

    //Allow only one connection to the socket
    listen(socketFileDescriptor, 1);

    clientIPAndPortLength = sizeof(clientIPAndPort);
    newSocketFileDescriptor = accept(socketFileDescriptor,
                                    (struct sockaddr*)&clientIPAndPort,
                                    &clientIPAndPortLength);

    if(newSocketFileDescriptor < 0){
        printErr("Error on accepting incoming connection request.");
    }

    //Clear buffer
    //TODO: Set max incoming job size from config
    memset(receivedText, 0, sizeof(receivedText));

    if(read(newSocketFileDescriptor, receivedText, 2047) < 0){
        printErr("Error reading message. Message too long?");
    }

    //TODO: Save job description to file
    printf("Listening on port %d", portNr);

    return true;
}

bool Server::stop(){
    //"Safe" closing of a socket. Courtesy of Joseph Quinsey, adapted as needed.
    //http://stackoverflow.com/questions/12730477/close-is-not-closing-socket-properly
    if (newSocketFileDescriptor >= 0 && socketFileDescriptor >= 0) {
        int err = 1;
        socklen_t len = sizeof err;
        if (-1 == getsockopt(socketFileDescriptor, SOL_SOCKET, SO_ERROR, (char *)&err, &len)){
            printErr("Error at socket level: \n SO_ERROR - socketFileDescriptor connection failed.");
        }

        if (-1 == getsockopt(newSocketFileDescriptor, SOL_SOCKET, SO_ERROR, (char *)&err, &len)){
            printErr("Error at socket level: \n SO_ERROR - newSocketFileDescriptor connection failed.");
        }

        if (err){
            // set errno to the socket SO_ERROR
            errno = err;
            printf("%d", err);
            return false;
        }

        if (shutdown(socketFileDescriptor, SHUT_RDWR) < 0){
            // SGI causes EINVAL
            if (errno != ENOTCONN && errno != EINVAL){
                printErr("Unknown error when closing socketFileDescriptor on server shutdown.");
                return false;
            }
        }

        if (shutdown(newSocketFileDescriptor, SHUT_RDWR) < 0){
            // SGI causes EINVAL
            if (errno != ENOTCONN && errno != EINVAL){
                printErr("Unknown error when closing newSocketFileDescriptor on server shutdown.");
                return false;
            }
        }

        if (close(newSocketFileDescriptor) < 0){
            printErr("Error closing socket file descriptor.");
            return false;
        }

        if(close(socketFileDescriptor) < 0){
            printErr("Error closing socket file descriptor.");
            return false;
        }
    }


    return true;
}

bool Server::restart(){
    if(stop() == false){
        if(start(portNr) == false){
            printErr("The server is not reacting. \n There was a severe error starting the server. Please try to restart manually or locally.");
        }
    }else{
        printErr("The server is not reacting. \n There was a severe error stopping the server. Please kill manually and restart jobs.");
    }
    return true;
}

bool Server::startDeamon()
{
    pid_t processID;
    //fork off the parent (this) process and kill it
    processID = fork();
    if(processID < 0){
        return false;
    }else{
        //kill parent
        exit(EXIT_SUCCESS);
    }

    if(setsid() < 0){
        exit(EXIT_FAILURE);
    }

    //TODO: signal handlers
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    //avoid this process being a session leader, fork again
    processID = fork();
    if(processID < 0){
        return false;
    }else{
        //kill parent
        exit(EXIT_SUCCESS);
    }

    //set file permissions
    umask(0);

    #ifdef DEBUG
    chdir("/home/felix/Development/mapedit/mapedit/build");
    #else
    chdir("/var/log");
    #endif // DEBUG

    for(int x = sysconf(_SC_OPEN_MAX); x >= 0; x--){
        close(x);
    }

    openlog("mapedit", LOG_PID, LOG_DAEMON);

    syslog(LOG_NOTICE, "Deamon started.\n");

    return true;
}

bool Server::stopDeamon()
{
    syslog(LOG_NOTICE, "Deamon terminated.\n");
    closelog();
}
/*------------------------------------------- NETWORKING ----------------------------------------------*/

/*--------------------------------------- RENDER MANAGEMENT -------------------------------------------*/

//Create a rendering context and start OpenGL
bool Server::startOGL()
{
    //Fetch function pointers to GLX functions
    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
    static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
    static glXMakeContextCurrentARBProc   glXMakeContextCurrentARB   = NULL;

    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
    glXMakeContextCurrentARB   = (glXMakeContextCurrentARBProc)   glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent"      );
    if(glXCreateContextAttribsARB == nullptr && glXMakeContextCurrentARB == nullptr){
        syslog(LOG_ERR, "Could not create rendering context: Extensions not supported.");
        return false;
    }

    //Open hidden display (never map onto screen)
    display = XOpenDisplay(NULL);

    if(display == nullptr){
        syslog(LOG_ERR, "Could not create hidden window.");
        return false;
    }

    //get available framebuffer configs (color bits, depth bits, etc.)
    static int visualAttribs[] = {None};
    int numberOfFramebufferConfigs = 0;
    fbConfigs = glXChooseFBConfig(display, DefaultScreen(display), visualAttribs, &numberOfFramebufferConfigs);

    //create context so we can start rendering
    //create tiny pbuffer, just so the rendering context is valid
    //the pbuffer won't be used, it's just so we don't have to create a window
    int pbufferAttribs[] = {
        GLX_PBUFFER_WIDTH, 32,
        GLX_PBUFFER_HEIGHT, 32,
        None
    };

    pbuffer = glXCreatePbuffer(display, fbConfigs[0], &pbufferAttribs);
    XFree(fbConfigs);
    XSync(display, False);

    if(!glXMakeContextCurrent(display, pbuffer, pbuffer, context)){
        syslog(LOG_ERR, "Could not make rendering context current.");
        return false;
    }

    return true;
}

//Fill the current framebuffer with whatever the renderer is spitting out
void getRenderedFrame(Job::Area& area){

}

//Send rendered frame over network (or to application window)
void sendRenderedFrame(){

}

bool Server::stopOGL(){
    glBindRenderbuffer(GL_RENDERBUFFER, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, NULL);

    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, context);
    XCloseDisplay(display);
}

/*--------------------------------------- RENDER MANAGEMENT -------------------------------------------*/

/*------------------------------------ COMMAND LINE ARGUMENTS -----------------------------------------*/

bool Server::loadConfig(const char* filePath)
{
    //Open file and get file size
    struct stat statbuf;

    if (stat(filePath, &statbuf) == -1) {
        //print error
        printErr(strerror(errno));
        return false;
    }

    FILE* configFile = fopen(filePath, "r");
    char* configFileBuf;
    fread(configFileBuf, statbuf.st_size, 1, configFile);

    unsigned int configFileLength = strlen(configFileBuf);
    if(configFileLength == 0){
        printErr("Empty config file.");
        return false;
    }

    fclose(configFile);

    char* line = strtok(strdup(configFileBuf), "\n");
    while(line) {
       printf("%s", line);
       line  = strtok(NULL, "\n");
    }

    return true;
}

void Server::printPortNumber()
{
    printf("Server running on port %d.", portNr);
}

void Server::handleCMDArgs(unsigned int argv, char** options)
{
    char* curArgument;

    for(unsigned int i = 1; i < argv; i++){
        printHelp();

        if((strcmp(curArgument, "-server") == 0 || strcmp(curArgument, "-s") == 0)){
            if(options[i+1] != nullptr){
                if(strcmp(options[i+1], "start") == 0){
                    start();
                    return;
                }
                if(strcmp(options[i+1], "stop") == 0){
                    stop();
                    return;
                }
                if(strcmp(options[i+1], "restart") == 0){
                    restart();
                    return;
                }
            }

            this->printServerCMDHelp();
            return;
        }

        if((strcmp(curArgument, "-config") == 0 || strcmp(curArgument, "-c") == 0)){
            if(options[i+1] != nullptr){
                loadConfig(options[i+1]);
                return;
            }else{
                printErr("No configuration file specified.");
            }
            this->printConfigCMDHelp();
            return;
        }

        if(strcmp(curArgument, "-help") == 0){
            if(options[i+1] != nullptr){
                loadConfig(options[i+1]);
            }else{
                printErr("Usage: -help [COMMAND].");
            }
        }

        printInfo(curArgument);
        //Fallback: print help
        printHelp();       return;
    }
}

void Server::printHelp()
{
    printf("print help\n");
}

void printServerCMDHelp()
{

}

void printConfigCMDHelp()
{

}

void printPortNumber()
{

}

/*------------------------------------ COMMAND LINE ARGUMENTS -----------------------------------------*/
