#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

struct OpenGLShader{
    OpenGLShader() { };
    OpenGLShader(const char* vertexShaderSource, const char* fragmentShaderSource);
    ~OpenGLShader() { };

    bool makeShader(const char* vertexShaderSource, const char* fragmentShaderSource);
    GLuint VAO, VBO, EBO, shaderProgram;
    GLint success;
    GLchar infoLog[512];
};
