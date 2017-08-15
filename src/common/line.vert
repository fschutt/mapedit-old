R"(

#version 130

//Shader idea by Konstantin Käfer
//https://www.mapbox.com/blog/drawing-antialiased-lines/

//Input: position and normal (each vertex)
attribute vec2	position;
attribute vec2  normal;

//Once per primitive
uniform float 	lineWidth;
uniform mat3 	modelViewMatrix;
uniform mat3 	projectionMatrix;

//TODO: Reprojection from coordinate system?
out vec4 		delta;

void main()
{
	gl_Normal 	= vec3(normal, 1.0);
	vec4 delta 	= vec4(normal * lineWidth, 0, 0);
	vec4 pos	= modelViewMatrix * vec4(position, 0, 1);
	gl_Position = projectionMatrix * (pos + delta);
}

)"

