#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Shader.h"
#include <string>
#include <iostream>
#include <fstream>

void Shader::loadFile(const char *fn, std::string &str)
{
	std::ifstream in(fn);

	if (!in.is_open())
	{
		std::cout << "The file of shader " << fn << "   cannot be opened\n";
		return;
	}
	
	char tmp[300];
	while (!in.eof())
	{
		in.getline(tmp, 300);
		str += tmp;
		str += '\n';
	}
}

GLuint Shader::loadShader(std::string &source, GLuint mode)
{
	GLuint sh = glCreateShader(mode);

	const char* shCode = source.c_str();

	GLint length = strlen(shCode);
	glShaderSource(sh, 1, &shCode, &length);
	glCompileShader(sh);

	GLint success;
	glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar InfoLog[1024];
		glGetShaderInfoLog(sh, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", mode, InfoLog);
	}
	return sh;
}

Shader::Shader(const char *vshh, const char *fshh)
{
	std::string source;

	loadFile(vshh, source);
	vsh = loadShader(source, GL_VERTEX_SHADER);
	source = "";

	loadFile(fshh, source);
	fsh = loadShader(source, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vsh);
	glAttachShader(program, fsh);

	glValidateProgram(program);
	glLinkProgram(program);
}

Shader::~Shader()
{
	glDetachShader(program, vsh);
	glDetachShader(program, fsh);
	glDeleteShader(vsh);
	glDeleteShader(fsh);
	glDeleteProgram(program);
}

void Shader::useShader()
{
	glUseProgram(program);
}

void Shader::delShader()
{
	glUseProgram(0);
}

GLuint Shader::getProgramId()
{
	return program;
}