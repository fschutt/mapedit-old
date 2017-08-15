#include "opengl_shader.h"
#include "print_log_console.h"
#include "errors.h"

OpenGLShader::OpenGLShader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    //NOTE(felix): May fail. Check infoLog for status and errors
    makeShader(vertexShaderSource, fragmentShaderSource);
}

//Function to compile shader
bool OpenGLShader::makeShader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check for compile time errors
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printErr((const char*) ERR_OPENGL_VERTEX_FAILED);
        printErr((const char*)infoLog);
        return false;
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check for compile time errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
          glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
          printErr(ERR_OPENGL_FRAGMENT_FAILED);
          printErr((const char*) infoLog);
          return false;
    }

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
          glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
          printErr(ERR_OPENGL_FRAGMENT_FAILED);
          printErr((char*)infoLog);
          return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return true;
}
