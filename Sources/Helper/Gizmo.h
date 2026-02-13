#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../Core/Shader.h"

// Gizmo with directions to help orient in space
class Gizmo
{
public:
    Gizmo();
    ~Gizmo();

    void init();                      
    void draw(const glm::mat4& VP);     

    void toggle(){ m_enabled = !m_enabled;}
    bool isEnabled()const{return m_enabled;}

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    Shader m_shader;
    bool m_enabled = false;
};
