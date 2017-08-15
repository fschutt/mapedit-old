#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <queue>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

#include "heightterrain.h"

struct Server
{
	int socketFileDescriptor;
	int newSocketFileDescriptor;
	unsigned int portNr;
	struct sockaddr_in serverIPAndPort;
	struct sockaddr_in clientIPAndPort;
	socklen_t clientIPAndPortLength;
	char receivedText[2048];
	int timeoutRetrySeconds = 5;

	//TODO: load from config
	constexpr static const char* heightTerrainFolderSRTM3Deg = "./";
	constexpr static const char* heightTerrainFolderSRTM30Deg = "./";

	//OpenGL context
	GLXContext context;
	//Pixel buffer, just for not having a window on-screen.
	//Rendering will be done in a framebuffer
	GLXPbuffer pbuffer;
	//Configuration for the framebuffer
	GLXFBConfig* fbConfigs;
	//Off-screen rendering buffer
	GLuint framebuffer;
	//Note: this does not have to be a physical display, only a connection to the x server
	Display* display;

	///FUNCTIONS
	///NETWORKING
	//Get server instance
	static Server& getInstance();
	//Start server to listen on port
	bool start(unsigned int portNumber = 0);
	//Stop server
    bool stop();
    //Restart server ( = stop + start)
    bool restart();
    //Forks current process to unbind it from a terminal window
	bool startDeamon();
	//Stop application deamon if there is a deamon running
	bool stopDeamon();

    ///OPENGL RENDERING
	//Start windowless OpenGL for offscreen rendering
	bool startOGL();
	//Request a rendered frame. Invokes the renderer
	void getRenderedFrame(Job::Area& area);
	//Sends the current framebuffer over the network connection to a remote
	//or local display
	void sendRenderedFrame();
	//Stop windowless OpenGL
	bool stopOGL();

	///LOGGING & HELP
	//Load server configuration on startup
    bool loadConfig(const char* filePath = "./mapedit.config");
	//Do command line help
	void handleCMDArgs(unsigned int argv, char** options);
	//Print general usage help
	void printHelp();

private:
	//help functions for printing help on console
	void printServerCMDHelp();
	void printConfigCMDHelp();
	void printPortNumber();

	static Server* instance;
	Server() {};
	~Server() {};

	//Server cannot be copied. Throws "use of deleted function" at compile time.
	Server(Server const&)			= delete;
	void operator=(Server const&)	= delete;
};
