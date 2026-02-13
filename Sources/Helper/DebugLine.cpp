#pragma once 

#include "DebugLine.h"

#include <glad/glad.h>

DebugLine::DebugLine()
{
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		sizeof(DebugVertex),
		(void*)0
	);

	glEnableVertexAttribArray(1); // color
	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE,
		sizeof(DebugVertex),
		(void*)offsetof(DebugVertex, color)
	);

	glBindVertexArray(0);

	m_shader.load("Resources/Shaders/DebugVert.glsl",
		"Resources/Shaders/DebugFrag.glsl");


}

void DebugLine::addLine(glm::vec3 a, glm::vec3 b, glm::vec3 color)
{
	m_vertices.push_back({ a, color });
	m_vertices.push_back({ b, color });
}

void DebugLine::draw(const glm::mat4& VP)
{
    if (m_vertices.empty())
        return;

    m_shader.use();
    m_shader.setMat4("uVP", VP);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        m_vertices.size() * sizeof(DebugVertex),
        m_vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vertices.size()));
    glBindVertexArray(0);
}
