#pragma once

#include "Mesh.h"

#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <limits>

// Load a mesh from an OFF file
bool Mesh::loadOFF(const std::string& filename)
{
    std::cout << "Start loading mesh <" << filename << ">\n";
    clear();

    std::ifstream in(filename);
    if (!in)
    {
        std::cerr << "[Mesh::loadOFF] Cannot open file: " << filename << "\n";
        return false;
    }

    std::string offString;
    unsigned int sizeV = 0, sizeT = 0, tmp = 0;

    in >> offString;
    if (offString != "OFF")
    {
        std::cerr << "[Mesh::loadOFF] Invalid OFF header\n";
        return false;
    }

    in >> sizeV >> sizeT >> tmp;

    m_positions.resize(sizeV);
    m_triangles.resize(sizeT);

    for (unsigned int i = 0; i < sizeV; ++i)
        in >> m_positions[i].x >> m_positions[i].y >> m_positions[i].z;

    int s;
    for (unsigned int i = 0; i < sizeT; ++i)
    {
        in >> s;
        if (s != 3)
        {
            std::cerr << "[Mesh::loadOFF] Non-triangular face\n";
            return false;
        }
        in >> m_triangles[i].x
            >> m_triangles[i].y
            >> m_triangles[i].z;
    }

    for (auto& p : m_positions)
    {
        // OFF (X, Y, Z) >> OpenGL (X, Z, -Y)
        float x = p.x;
        float y = p.y;
        float z = p.z;

        p.x = x;
        p.y = z;
        p.z = -y;
    }


    in.close();

    m_normals.resize(m_positions.size(), glm::vec3(0.f));

    recomputePerVertexNormals();

    // compute local-space AABB from vertex positions
    computeAABB();

    return true;
}

// Upload the mesh data to GPU (VAO, VBO, EBO)
void Mesh::upload()
{
    if (m_positions.empty() || m_triangles.empty())
        return;

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
    };

    std::vector<Vertex> vertices(m_positions.size());
    for (size_t i = 0; i < m_positions.size(); ++i)
    {
        vertices[i].pos = m_positions[i];
        vertices[i].normal = m_normals[i];
    }

    std::vector<unsigned int> indices;
    indices.reserve(m_triangles.size() * 3);
    for (const auto& t : m_triangles)
    {
        indices.push_back(t.x);
        indices.push_back(t.y);
        indices.push_back(t.z);
    }

    m_indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)0);

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);
}

// Render the mesh
void Mesh::draw() const
{
    if (m_vao == 0) return;

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Clear the mesh data
void Mesh::clear()
{
    m_positions.clear();
    m_triangles.clear();
    m_normals.clear();
    m_indexCount = 0;
    // reset AABB
    m_aabb.min = glm::vec3(std::numeric_limits<float>::infinity());
    m_aabb.max = glm::vec3(-std::numeric_limits<float>::infinity());
}

// Create a simple quad mesh of given size
void Mesh::createQuad(float size)
{
    clear();

    float s = size * 0.5f;

    m_positions = {
        {-s, 0.f, -s},
        { s, 0.f, -s},
        { s, 0.f,  s},
        {-s, 0.f,  s}
    };

    m_triangles = {
        {0, 1, 2},
        {2, 3, 0}
    };

    m_normals.resize(4, glm::vec3(0.f, 1.f, 0.f));

    // update AABB for quad
    computeAABB();
}

// Recompute per-vertex normals based on face normals
void Mesh::recomputePerVertexNormals()
{
    std::fill(m_normals.begin(), m_normals.end(), glm::vec3(0.f));

    for (const auto& tri : m_triangles)
    {
        glm::vec3 p0 = m_positions[tri.x];
        glm::vec3 p1 = m_positions[tri.y];
        glm::vec3 p2 = m_positions[tri.z];

        glm::vec3 n = glm::normalize(glm::cross(p1 - p0, p2 - p0));

        m_normals[tri.x] += n;
        m_normals[tri.y] += n;
        m_normals[tri.z] += n;
    }

    for (auto& n : m_normals)
        n = glm::normalize(n);
}

// compute mesh bounding sphere (center and radius)
//void Mesh::computeBoundingSphere(glm::vec3& center, float& radius)
//{
//    center = glm::vec3(0.0);
//    radius = 0.f;
//
//    for (const auto& p : m_positions) {
//        center += p;
//    }
//    center /= m_positions.size();
//
//    for (const auto& p : m_positions) {
//        radius = glm::max(radius, glm::distance(p, center));
//    }
//}

//bool Mesh::intersectBoundingSphere(const Ray& ray)
//{
//    glm::vec3 center;
//    float radius;
//    computeBoundingSphere(center, radius);
//
//    glm::vec3 oc = ray.origin - center;
//
//    float a = glm::dot(ray.direction, ray.direction);
//    float b = 2.0f * glm::dot(oc, ray.direction);
//    float c = glm::dot(oc, oc) - radius * radius;
//
//    float disc = b * b - 4 * a * c;
//    if (disc < 0.0f)
//        return false;
//
//    float sqrtD = std::sqrt(disc);
//    float t0 = (-b - sqrtD) / (2 * a);
//    float t1 = (-b + sqrtD) / (2 * a);
//
//    return (t0 > 0.0f || t1 > 0.0f);
//}

//// Intersect ray with all triangles of the mesh
//bool Mesh::intersectTriangles(const Ray& ray, Hit& hit) {
//
//    bool hitSomething = false;
//    hit.t = std::numeric_limits<float>::infinity();
//
//    glm::mat4 M = computeTransformMatrix();
//    glm::mat4 invM = glm::inverse(M);
//
//    Ray localRay;
//    localRay.origin =
//        glm::vec3(invM * glm::vec4(ray.origin, 1.0f));
//
//    localRay.direction =
//        glm::normalize(glm::vec3(invM * glm::vec4(ray.direction, 0.0f)));
//
//    for (const auto& tri : m_triangles)
//    {
//        glm::vec3 v0 = m_positions[tri.x];
//        glm::vec3 v1 = m_positions[tri.y];
//        glm::vec3 v2 = m_positions[tri.z];
//
//        // Möller–Trumbore intersection algorithm
//        glm::vec3 edge1 = v1 - v0;
//        glm::vec3 edge2 = v2 - v0;
//        glm::vec3 h = glm::cross(localRay.direction, edge2);
//        float a = glm::dot(edge1, h);
//
//        if (std::abs(a) < 1e-8) // ray is parallel to triangle 
//            continue;
//
//        float f = 1.0f / a;
//        glm::vec3 s = localRay.origin - v0;
//        float u = f * glm::dot(s, h);
//
//        if (u < 0.0f || u > 1.0f)   // intersection lies outside the triangle
//            continue;
//
//        glm::vec3 q = glm::cross(s, edge1);
//        float v = f * glm::dot(localRay.direction, q);
//
//        if (v < 0.0f || u + v > 1.0f)   // intersection lies outside the triangle
//            continue;
//
//        float t = f * glm::dot(edge2, q);
//        if (t > 1e-8 && t < hit.t) // ray intersection
//        {
//            hit.t = t;
//            hit.position = localRay.origin + localRay.direction * t;
//            hit.normal = glm::normalize(glm::cross(edge1, edge2));
//            hit.color = m_color;
//            hitSomething = true;
//            //std::cout << "Hit at t=" << hit.t << "\n";
//        }
//    }
//    hit.position = glm::vec3(M * glm::vec4(hit.position, 1.0f));
//
//    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(M)));
//    hit.normal = glm::normalize(normalMat * hit.normal);
//
//	return hitSomething;
//}


//bool Mesh::intersect(const Ray& ray, Hit& hit)
//{
//    glm::mat4 M = computeTransformMatrix();
//    glm::mat4 invM = glm::inverse(M);
//
//    Ray localRay;
//    localRay.origin =
//        glm::vec3(invM * glm::vec4(ray.origin, 1.0f));
//
//    localRay.direction =
//        glm::normalize(glm::vec3(invM * glm::vec4(ray.direction, 0.0f)));
//
//    // use AABB in local space 
//    if (!m_aabb.intersect(localRay))
//        return false;
//
//    return intersectTriangles(ray, hit);
//}

// compute AABB in local space from vertex positions
void Mesh::computeAABB()
{
    if (m_positions.empty()) {
        m_aabb.min = glm::vec3(0.0f);
        m_aabb.max = glm::vec3(0.0f);
        return;
    }

    m_aabb.min = m_positions[0];
    m_aabb.max = m_positions[0];

    for (const auto& p : m_positions) {
        m_aabb.min = glm::min(m_aabb.min, p);
        m_aabb.max = glm::max(m_aabb.max, p);
    }
}


	
