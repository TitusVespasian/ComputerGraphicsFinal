
#include "water\Program_w.hpp"

Program_w::Program_w()
{
    amount = new unsigned;
    *amount = 1;
}

Program_w::Program_w(const Shader_w& vertex, const Shader_w& fragment)
{
    programId = glCreateProgram();
    glAttachShader(programId, vertex.getId());
    glAttachShader(programId, fragment.getId());
    link();
    vertex.clear();
    fragment.clear();
    amount = new unsigned;
    *amount = 1;
}

Program_w::Program_w(const std::string& vertFileName, const std::string& fragFileName)
{
    programId = glCreateProgram();
    Shader_w vertex = createVertexShader(vertFileName);
    Shader_w fragment = createFragmentShader(fragFileName);
    glAttachShader(programId, vertex.getId());
    glAttachShader(programId, fragment.getId());
    link();
    vertex.clear();
    fragment.clear();
    amount = new unsigned;
    *amount = 1;
}

Program_w::Program_w(const Program_w& program)
{
    swap(program);
}

Program_w& Program_w::operator=(const Program_w& program)
{
    clear();
    swap(program);
    return *this;
}

void Program_w::swap(const Program_w& program)
{
    programId = program.programId;
    amount = program.amount;
    *amount = *amount + 1;
}

void Program_w::clear()
{
    *amount = *amount - 1;
    if (*amount == 0)
    {
        delete amount;
        if (programId != 0)
            glDeleteProgram(programId);
    }
}

Program_w::~Program_w()
{
    clear();
}

std::string Program_w::getLinkMessageErrorAndClear() const
{
    int length;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
    char* message = new char[length];

    glGetProgramInfoLog(programId, length, &length, message);
    glDeleteProgram(programId);

    std::string finalMess = message;
    delete[] message;
    return finalMess;
}

void Program_w::create()
{
    if (programId != 0)
        throw std::runtime_error("Can't create exists program");
    programId = glCreateProgram();
}

void Program_w::attachShader(const Shader_w& shader) const
{
    if (programId == 0)
        throw std::runtime_error("Can't attach Shader_w to empty program");
    glAttachShader(programId, shader.getId());
    shader.clear();
}

void Program_w::link() const
{
    if (programId == 0)
        throw std::runtime_error("Can't link empty program");
    glLinkProgram(programId);
    int success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
        throw std::runtime_error(getLinkMessageErrorAndClear());
}

unsigned Program_w::getUniformId(const char* name) const
{
    return glGetUniformLocation(programId, name);
}

void Program_w::setInt(const char* name, int i) const
{
    glUniform1i(getUniformId(name), i);
}

void Program_w::setFloat(const char* name, float f) const
{
    glUniform1f(getUniformId(name), f);
}

void Program_w::setVec2(const char* name, const glm::vec2& vec) const
{
    glUniform2fv(getUniformId(name), 1, glm::value_ptr(vec));
}

void Program_w::setVec3(const char* name, const glm::vec3& vec) const
{
    glUniform3fv(getUniformId(name), 1, glm::value_ptr(vec));
}

void Program_w::setVec4(const char* name, const glm::vec4& vec) const
{
    glUniform4fv(getUniformId(name), 1, glm::value_ptr(vec));
}

void Program_w::setMat2(const char* name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(getUniformId(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Program_w::setMat3(const char* name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(getUniformId(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Program_w::setMat4(const char* name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(getUniformId(name), 1, GL_FALSE, glm::value_ptr(mat));
}

Shader_w createVertexShader(const std::string& fileName)
{
    return Shader_w(fileName, GL_VERTEX_SHADER);
}

Shader_w createFragmentShader(const std::string& fileName)
{
    return Shader_w(fileName, GL_FRAGMENT_SHADER);
}

Shader_w createGeometryShader(const std::string& fileName)
{
    return Shader_w(fileName, GL_GEOMETRY_SHADER);
}

Shader_w createTessalationControlShader(const std::string& fileName)
{
    return Shader_w(fileName, GL_TESS_CONTROL_SHADER);
}

Shader_w createTessalationEvaluationShader(const std::string& fileName)
{
    return Shader_w(fileName, GL_TESS_EVALUATION_SHADER);
}
