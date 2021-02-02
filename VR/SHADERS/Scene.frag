#version 410 core
in vec3 color1;
out vec4 outputColor;
void main()
{
		outputColor = vec4(color1, 1);
}