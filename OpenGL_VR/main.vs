attribute vec3 pos;
attribute vec3 color;
varying vec3 color1;
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

void main( )
{
	  color1 = color;
	  gl_Position = projection * model * view * vec4(pos, 1);
}