uniform sampler2D tex;
varying vec2 uv11;
void main( )
{
	gl_FragColor = texture2D(tex, uv11);
}