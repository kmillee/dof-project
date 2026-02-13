#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Base class to represent spatially embedded entities in the scene
class Transform {
public:
	Transform () : m_translation (0.0), m_rotation (0.0), m_scale (1.0) {}
	virtual ~Transform () {}

	inline const glm::vec3 getTranslation () const { return m_translation; }
	inline void setTranslation (const glm::vec3 & t) { m_translation = t; }
	inline const glm::vec3 getRotation () const { return m_rotation; }
	inline void setRotation (const glm::vec3 & r) { m_rotation = r; }
	inline float getScale () const { return m_scale; }
	inline void setScale (float s) { m_scale = s; }

	inline glm::mat4 computeTransformMatrix () const {
		glm::mat4 M(1.0f);
		M = glm::translate(M, m_translation);
		M = glm::rotate(M, m_rotation.x, { 1,0,0 });
		M = glm::rotate(M, m_rotation.y, { 0,1,0 });
		M = glm::rotate(M, m_rotation.z, { 0,0,1 });
		M = glm::scale(M, glm::vec3(m_scale));
		return M;
	}

private:
	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	float m_scale;
};
