
#ifndef PROGRAM_W_HPP
#define PROGRAM_HPP 1
#include "water\Shader_w.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Shader_w createVertexShader(const std::string& fileName);
Shader_w createFragmentShader(const std::string& fileName);
Shader_w createGeometryShader(const std::string& fileName);
Shader_w createTessalationControlShader(const std::string& fileName);
Shader_w createTessalationEvaluationShader(const std::string& fileName);

class Program_w
{
	unsigned programId = 0;
	unsigned* amount;

	std::string getLinkMessageErrorAndClear() const;
	unsigned getUniformId(const char* name) const;

	void swap(const Program_w& program);
	void clear();

public:
	Program_w();
	Program_w(const Program_w& program);
	Program_w& operator=(const Program_w& program);
	Program_w(const Shader_w& vertex, const Shader_w& fragment);
	Program_w(const std::string& vertFileName, const std::string& fragFileName);
	~Program_w();

	void create();
	void link() const;
	void attachShader(const Shader_w& shader) const;

	void use() const { glUseProgram(programId); }

	void setInt(const char* name, int i) const;
	void setFloat(const char* name, float f) const;
	void setVec2(const char* name, const glm::vec2& vec) const;
	void setVec3(const char* name, const glm::vec3& vec) const;
	void setVec4(const char* name, const glm::vec4& vec) const;
	void setMat2(const char* name, const glm::mat2& mat) const;
	void setMat3(const char* name, const glm::mat3& mat) const;
	void setMat4(const char* name, const glm::mat4& mat) const;

	unsigned getId() const { return programId; }
};
#endif
