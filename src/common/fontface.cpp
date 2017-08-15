#include "fontface.h"
#include "print_log_console.h"
#include "errors.h"
#include "messages.h"

FontFace::FontFace(const char* pathRelativeToExecutable, unsigned int size)
{
	currentSize = size;

	if(strlen(pathRelativeToExecutable) <= 0){
        printErr("No font specified to load.");
        return;
	}

    if(FT_Init_FreeType(&library))
    {
        printErr(ERR_FT_INIT_FAILED);
        return;
    }

    if(FT_New_Face(library, pathRelativeToExecutable, 0, &face))
    {
        printErr(ERR_FT_INIT_FAILED);
        return;
    }

    //Render bitmaps
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //0 = automatic width
    FT_Set_Pixel_Sizes(face, 0, currentSize);

    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            printErr(ERR_FT_GLYPH_LOAD_FAILED);
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };

        characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

FontFace::~FontFace(){

}
