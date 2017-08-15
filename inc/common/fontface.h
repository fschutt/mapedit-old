#pragma once

#include <freetype/ftsnames.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>

#include "glm/glm.hpp"

struct Character
{
    GLuint              textureID;
    glm::ivec2          size;
    glm::ivec2          bearing;
    long int            advance;
};

struct FontFace
{
    FT_SfntName name;
    FT_Library library;
    FT_Face face;

    const char* filePath;

    unsigned int currentSize;	//current size of the font

    //Rendered character map
    std::map<GLchar, Character> characters;

    FontFace();
	FontFace(const char* pathRelativeToExecutable, unsigned int size);
	~FontFace();
};
