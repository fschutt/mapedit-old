#pragma once

#include <map>

#include "glm/glm.hpp"
#include "fontface.h"

struct Renderer;

struct StaticText
{
    //TODO: check current font for nullptr
    StaticText(const char* text);
    StaticText(const char* text, unsigned int size);
    StaticText(const char* text, unsigned int size, FontFace* font);
    StaticText(const char* text, unsigned int size, FontFace* font, glm::vec3 color);
    StaticText(const char* text, unsigned int size, FontFace* font, glm::vec3 color, double rotation);
    ~StaticText() { };

    //TODO(felix): font and rotation
    FontFace* font;
    double rotation = 0;
    glm::vec3 color;
    unsigned int currentSize; //text size
    const char* text;

    Renderer* renderer;
    static GLuint characterIndices[];
    GLuint VAO, VBO, EBO;
    void draw(const char* text, GLfloat offsetX, GLfloat offsetY, GLfloat scale, glm::vec3 color, glm::mat4 projection, GLuint& shaderProgram);
};
