#pragma once
//NOTE(felix):	File separated for translation purposes.
//		Add language file with translated strings here
//		and define the language you want to use in the
//		build script

#ifdef LANG
	#if LANG==EN_US
		#include "errors_en_us.h"
	#endif
#else
	#error "No language defined for errors."
#endif

#define	ERR_X_SERVER_NO_CONNECT                 E00001
#define	ERR_X_MONITOR_MODE_NOT_SUPPORTED		E00002
#define	ERR_X_SERVER_BAD_ALLOC			    	E00003
#define	ERR_X_SERVER_BAD_WINDOW				    E00004
#define	ERR_X_NO_OPENGL					        E00005
#define	ERR_X_NO_GLEW					        E00006
#define	ERR_UNKNOWN					            E00007
#define	ERR_OPENGL_VERTEX_FAILED	    		E00008
#define	ERR_OPENGL_FRAGMENT_FAILED		    	E00009
#define	ERR_OPENGL_LINKING_FAILED			    E00010
#define	ERR_OPENGL_VERSION_INVALID			    E00011
#define ERR_FT_INIT_FAILED                      E00012
#define ERR_FT_GLYPH_LOAD_FAILED                E00013
