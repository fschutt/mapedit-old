#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

struct Quad{
    Quad();
    ~Quad();
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;

    GLfloat vertices[6] = {
        0, 1, 3,
        1, 2, 3
    };

    GLfloat indices[12] = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    void init();
    void draw();
};
