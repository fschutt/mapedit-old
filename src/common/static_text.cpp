#include <map>
#include "static_text.h"
#include "glm/ext.hpp"

#include "print_log_console.h"
#include "errors.h"
#include "opengl_shader.h"

GLuint StaticText::characterIndices[] = {
    3, 1, 0,                            //Triangle 1
    3, 2, 1                             //Triangle 2
};

StaticText::StaticText(const char* text, unsigned int sz)
    : text(text), currentSize(sz)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void StaticText::draw(const char* text, GLfloat offsetX, GLfloat offsetY, GLfloat scale, glm::vec3 color, glm::mat4 projection, GLuint& shaderProgram)
{
    //Generate buffers
    glBindVertexArray(VAO);

    //Send text color and projection matrix as uniforms
    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //Activate a texture to use for sampling
    glActiveTexture(GL_TEXTURE0);

    //Each frame, for each character in the text
    for(GLubyte i=0; text[i] != 0; i++)
    {
        //Load character from list (this works)
        Character ch = font->characters.at(text[i]);

        //Calculate left bottom point of character
        GLfloat xpos = offsetX + (ch.bearing.x * scale);
        GLfloat ypos = offsetY - ((ch.size.y - ch.bearing.y) * scale);

        //Calculate actual width and height of character
        GLfloat w = ch.size.x * scale;
        GLfloat h = ch.size.y * scale;

        //Calculate vertices for VBO
        GLfloat characterVertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };

        //Bind the texture to the quad, shader program is already bound
        glBindTexture(GL_TEXTURE_2D, ch.textureID);

       //Bind VBO and put the characterVertices into GPU-visible memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //Fill VBO with calculated data
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(characterVertices), &characterVertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //Draw with characterIndices as index buffer
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        //Glvoid*0 takes the last bound ELEMENT_ARRAY_BUFFER
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //advance x for the next character
        //bitshift by 6 to get the value in pixels (2^6 = 64)
        offsetX += (ch.advance >> 6) * scale;
    }

    //Unbind all buffers and textures
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}
