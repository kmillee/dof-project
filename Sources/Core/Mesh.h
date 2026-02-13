#pragma once

#include "Transform.h"
#include "../RayHit/Ray.h"
#include "../RayHit/Hit.h"
#include "../RayHit/AABB.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>

// Simple mesh class for loading and rendering 3D models from OFF files
class Mesh : public Transform {
public:
    // ---- Setup ----
    bool loadOFF(const std::string& path);
    void upload();
    void clear();

    // ---- Geometry helpers ----
    void computeAABB();
	void createQuad(float size);
    void recomputePerVertexNormals();
	void computeBoundingSphere(glm::vec3& center, float& radius);

    // ---- Rendering ----
    void draw() const;

    // ---- Ray intersection ----
    bool intersectBoundingSphere(const Ray& ray);
    bool intersectTriangles(const Ray & ray, Hit & hit);
    bool intersect(const Ray& ray, Hit& hit);

    // ---- Getters/Setters -----
    inline const std::vector<glm::vec3>& vertexPositions() const { return m_positions; }
    inline std::vector<glm::vec3>& vertexPositions() { return m_positions; }
    inline const std::vector<glm::vec3>& vertexNormals() const { return m_normals; }
    inline std::vector<glm::vec3>& vertexNormals() { return m_normals; }
    inline const std::vector<glm::uvec3>& triangleIndices() const { return m_triangles; }
    inline std::vector<glm::uvec3>& triangleIndices() { return m_triangles; }
	inline glm::mat3& normalMat() { return m_normalMat; }
	glm::vec3& color() { return m_color; }
	void setColor(const glm::vec3& color) { m_color = color; }
    const AABB& boundingBox() const { return m_aabb; }


private:
    unsigned int m_vao = 0, m_vbo = 0, m_ebo = 0;
    unsigned int m_indexCount = 0;

    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::uvec3> m_triangles;

	glm::vec3 m_color = glm::vec3(0.5f, 0.5f, 0.5f); // default gray color
	glm::mat3 m_normalMat = glm::mat3(0.0f); // normal matrix for transforming normals
	AABB m_aabb;
};
