#ifndef SHADER_H
#define SHADER_H
#include <iostream>
#include <string>
#include <vector>

class Shader
{
public:
	GLuint vsh, fsh, program;
	void loadFile(const char *fn, std::string &str);
	GLuint loadShader(std::string &source, GLuint mode);

public:
	shader(const char *vshh, const char *fshh);
	~shader();
	void useShader();
	void delShader();
	GLuint getProgramId();

};

#endif