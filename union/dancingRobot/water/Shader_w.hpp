
#ifndef SHADER_W_HPP
#define SHADER_W_HPP 1
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glad/glad.h> 

class Shader_w
{
	unsigned shaderId;
	GLenum type;

	std::string getSource(const std::string& fileName);
	std::string getCompileMessageErrorAndClear();
public:
	Shader_w(const std::string& fileName, GLenum t);
	void clear() const { glDeleteShader(shaderId); }

	unsigned getId() const { return shaderId; }
	GLenum getType() const { return type; }
};
#endif
