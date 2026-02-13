#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../Core/Shader.h"

#include <vector>

struct Line
{
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 color;
};

struct DebugVertex
{
    glm::vec3 position;
    glm::vec3 color;
};

// Simple class to draw debug lines in 3D
class DebugLine
{
public:
    DebugLine();
    void addLine(glm::vec3 a, glm::vec3 b, glm::vec3 c);
    void clear() { m_vertices.clear(); }
    void draw(const glm::mat4& VP);

private:
    std::vector<DebugVertex> m_vertices;
    std::vector<Line> m_lines;
    GLuint m_vao;
    GLuint m_vbo;
    Shader m_shader;
};
