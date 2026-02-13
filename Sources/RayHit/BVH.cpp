#pragma once

#include "BVH.h"

#include <algorithm>
#include <iostream>

// Code taken and adapted from previous class
namespace {
    // helper to get world-space position of triangle vertex
    inline glm::vec3 worldPosOfVertex(const std::shared_ptr<Mesh>& mesh, uint32_t vidx) {
        const auto& P = mesh->vertexPositions();
        glm::mat4 M = mesh->computeTransformMatrix();
        return glm::vec3(M * glm::vec4(P[vidx], 1.0f));
    }
}

// Bounding Volume Hierarchy (BVH)
BVH::BVH(const std::vector<std::shared_ptr<Mesh>>& meshes) : m_left(nullptr), m_right(nullptr) {
    auto idxSet = makeIndexPairSet(meshes);
    if (!idxSet.empty())
        *this = BVH(meshes, idxSet, 0, idxSet.size());
}

std::vector<std::pair<size_t,size_t>> BVH::makeIndexPairSet(const std::vector<std::shared_ptr<Mesh>>& meshes) {
    std::vector<std::pair<size_t,size_t>> indexPairSet;
    for (size_t mi = 0; mi < meshes.size(); ++mi) {
        const auto& mesh = meshes[mi];
        const auto& tris = mesh->triangleIndices();
        for (size_t ti = 0; ti < tris.size(); ++ti)
            indexPairSet.emplace_back(mi, ti);
    }
    return indexPairSet;
}

// Recursive constructor
BVH::BVH(const std::vector<std::shared_ptr<Mesh>>& meshes,
         std::vector<std::pair<size_t,size_t>>& indexPairSet,
         size_t begin,
         size_t end)
    : m_left(nullptr), m_right(nullptr), m_meshIndex(size_t(-1)), m_triangleIndex(size_t(-1))
{
    //std::cout << "BVH node: build called with [" << begin << "," << end << "]\n";
    if (begin >= end) return;

    // build bbox from triangles in [begin,end)
    m_bbox.min = glm::vec3(std::numeric_limits<float>::infinity());
    m_bbox.max = glm::vec3(-std::numeric_limits<float>::infinity());

    for (size_t i = begin; i < end; ++i) {
        const auto& ip = indexPairSet[i];
        const auto& mesh = meshes[ip.first];
        const auto& tri = mesh->triangleIndices()[ip.second];
        for (int v = 0; v < 3; ++v) {
            glm::vec3 p = worldPosOfVertex(mesh, tri[v]);
            m_bbox.min = glm::min(m_bbox.min, p);
            m_bbox.max = glm::max(m_bbox.max, p);
        }
    }

    size_t count = end - begin;

    // - if count == 1 -> leaf storing that triangle
    // - if count == 2 -> create two leaf children so both triangles are represented
    if (count == 1) {
        m_meshIndex = indexPairSet[begin].first;
        m_triangleIndex = indexPairSet[begin].second;
        m_left = m_right = nullptr;
        return;
    } else if (count == 2) {
        size_t mid = begin + 1;
        // create two leaf children (each will have count==1 and thus store that triangle)
        m_left = new BVH(meshes, indexPairSet, begin, mid);
        m_right = new BVH(meshes, indexPairSet, mid, end);
        return;
    }

    // compute centroid bbox and split along largest axis
    glm::vec3 cmin(std::numeric_limits<float>::infinity());
    glm::vec3 cmax(-std::numeric_limits<float>::infinity());
    for (size_t i = begin; i < end; ++i) {
        const auto& ip = indexPairSet[i];
        const auto& mesh = meshes[ip.first];
        const auto& tri = mesh->triangleIndices()[ip.second];
        glm::vec3 centroid = (worldPosOfVertex(mesh, tri[0]) + worldPosOfVertex(mesh, tri[1]) + worldPosOfVertex(mesh, tri[2])) / 3.0f;
        cmin = glm::min(cmin, centroid);
        cmax = glm::max(cmax, centroid);
    }

    glm::vec3 diag = cmax - cmin;
    int axis = 0;
    if (diag.y > diag.x && diag.y >= diag.z) axis = 1;
    else if (diag.z > diag.x && diag.z > diag.y) axis = 2;

    auto midIter = indexPairSet.begin() + static_cast<std::ptrdiff_t>(begin + count / 2);
    std::nth_element(indexPairSet.begin() + static_cast<std::ptrdiff_t>(begin),
                     midIter,
                     indexPairSet.begin() + static_cast<std::ptrdiff_t>(end),
                     [&](const std::pair<size_t,size_t>& a, const std::pair<size_t,size_t>& b) {
                         const auto& ma = meshes[a.first];
                         const auto& triA = ma->triangleIndices()[a.second];
                         glm::vec3 ca = (worldPosOfVertex(ma, triA[0]) + worldPosOfVertex(ma, triA[1]) + worldPosOfVertex(ma, triA[2])) / 3.0f;
                         const auto& mb = meshes[b.first];
                         const auto& triB = mb->triangleIndices()[b.second];
                         glm::vec3 cb = (worldPosOfVertex(mb, triB[0]) + worldPosOfVertex(mb, triB[1]) + worldPosOfVertex(mb, triB[2])) / 3.0f;
                         return ca[axis] < cb[axis];
                     });

    size_t mid = begin + count / 2;
    m_left = new BVH(meshes, indexPairSet, begin, mid);
    m_right = new BVH(meshes, indexPairSet, mid, end);
}

BVH::BVH(const BVH& other) : m_left(nullptr), m_right(nullptr),
    m_bbox(other.m_bbox), m_meshIndex(other.m_meshIndex), m_triangleIndex(other.m_triangleIndex)
{
    if (other.m_left) m_left = new BVH(*other.m_left);
    if (other.m_right) m_right = new BVH(*other.m_right);
}

BVH::~BVH() {
    delete m_left;
    delete m_right;
    m_left = m_right = nullptr;
}

BVH& BVH::operator=(const BVH& other) {
    if (this == &other) return *this;
    delete m_left; delete m_right;
    m_left = m_right = nullptr;
    m_bbox = other.m_bbox;
    m_meshIndex = other.m_meshIndex;
    m_triangleIndex = other.m_triangleIndex;
    if (other.m_left) m_left = new BVH(*other.m_left);
    if (other.m_right) m_right = new BVH(*other.m_right);
    return *this;
}


void BVH::traverse(const Ray& ray, TriangleVisitor& visitor) const {
    if (!m_bbox.intersect(ray))
        return;

    if (isLeaf()) {
        if (m_meshIndex != size_t(-1)) {
            visitor.visit(m_meshIndex, m_triangleIndex);
        }
        return;
    }

    if (m_left)  m_left->traverse(ray, visitor);
    if (m_right) m_right->traverse(ray, visitor);
}


