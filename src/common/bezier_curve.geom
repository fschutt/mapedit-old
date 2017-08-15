R"(

//Since Linux has troubles doing OpenGL 3.3, this file is just for future use,
//should OGL 3.2 or 3.3 be available on the target system. For now it is better to
//implement this on the CPU, since we won't draw many bezier handles / points anyway

#version 130
out vec4 color;


void main() {
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}

)"
