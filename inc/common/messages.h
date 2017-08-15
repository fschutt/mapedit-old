#pragma once

//NOTE(felix):	File separated for translation purposes.
//		Add language file with translated strings here
//		and define the language you want to use in the
//		build script

#ifdef LANG
	#if LANG==EN_US
		#include "messages_en_us.h"
	#endif
#else
	#error "No language defined for errors."
#endif

#define	MSG_OPENGL_INIT_SUCCESS				M00001
#define	MSG_MENU_FILE					M00002
#define	MSG_MENU_OPEN					M00003
