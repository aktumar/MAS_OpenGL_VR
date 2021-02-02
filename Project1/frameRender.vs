attribute vec3 pos;
attribute vec2 uv;
varying vec2 uv11;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main( )
{
    uv11 = uv;
	gl_Position = projection * model * view * vec4(pos, 1);
}