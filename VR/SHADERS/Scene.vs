#version 410
uniform mat4 matrix;
uniform mat4 view;
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;
out vec3 color1;
void main()
{
	color1 = color;
	gl_Position = matrix * position;
}