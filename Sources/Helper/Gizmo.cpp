#pragma once

#include "Gizmo.h"

Gizmo::Gizmo() { this->init();}

Gizmo::~Gizmo()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void Gizmo::init()
{
    float axis = 2.f;

    float vertices[] = {
        0,0,0,  axis,0,0,   // X
        0,0,0,  0,axis,0,   // Y
        0,0,0,  0,0,axis    // Z
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    m_shader.load("Resources/Shaders/Debug/GizmoVert.glsl", "Resources/Shaders/Debug/GizmoFrag.glsl");
}

void Gizmo::draw(const glm::mat4& VP)
{
    if (!m_enabled) return;

    glDisable(GL_DEPTH_TEST);

    m_shader.use();
    m_shader.setMat4("uMVP", VP);

    glBindVertexArray(m_vao);

    m_shader.setVec3("uColor", { 1,0,0 });
    glDrawArrays(GL_LINES, 0, 2);

    m_shader.setVec3("uColor", { 0,1,0 });
    glDrawArrays(GL_LINES, 2, 2);

    m_shader.setVec3("uColor", { 0,0,1 });
    glDrawArrays(GL_LINES, 4, 2);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}


