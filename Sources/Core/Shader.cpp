#pragma once

#include "Shader.h"

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>



// Helper function to read shader source code from file
static std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Load and compile vertex and fragment shaders from files
bool Shader::load(const std::string& vsPath, const std::string& fsPath)
{
    std::string vsSrc = readFile(vsPath);
    std::string fsSrc = readFile(fsPath);

    const char* vsrc = vsSrc.c_str();
    const char* fsrc = fsSrc.c_str();

    // --- Vertex shader ---
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsrc, nullptr);
    glCompileShader(vs);

    GLint success = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(vs, 1024, nullptr, infoLog);
        std::cerr << "[Shader] Vertex compile error:\n"
            << infoLog << std::endl;
        glDeleteShader(vs);
        return false;
    }

    // --- Fragment shader ---
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsrc, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(fs, 1024, nullptr, infoLog);
        std::cerr << "[Shader] Fragment compile error:\n"
            << infoLog << std::endl;
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    // --- Program link ---
    m_id = glCreateProgram();
    glAttachShader(m_id, vs);
    glAttachShader(m_id, fs);
    glLinkProgram(m_id);

    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(m_id, 1024, nullptr, infoLog);
        std::cerr << "[Shader] Program link error:\n"
            << infoLog << std::endl;

        glDeleteProgram(m_id);
        glDeleteShader(vs);
        glDeleteShader(fs);
        m_id = 0;
        return false;
    }

    // Shaders no longer needed after linking
    glDeleteShader(vs);
    glDeleteShader(fs);

    return true;
}


void Shader::use() const
{
    glUseProgram(m_id);
}

// Set a 4x4 matrix uniform in the shader
void Shader::setMat4(const std::string& name, const glm::mat4& m) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()),
        1, GL_FALSE, &m[0][0]);
}

void Shader::setVec4(const std::string& name, const glm::vec3& v) const
{
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()),
        1, &v[0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& v) const
{
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()),
        1, &v[0]);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}