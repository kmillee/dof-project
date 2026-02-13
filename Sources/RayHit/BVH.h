#pragma once

#include "AABB.h"
#include "Ray.h"
#include "TriangleVisitor.h"
#include "../Core/Mesh.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <utility>
#include <limits>


class BVH {
public:
    BVH(const std::vector<std::shared_ptr<Mesh>>& meshes);
    BVH(const BVH& other);
	~BVH();
    BVH& operator=(const BVH& other);

    inline bool isLeaf() const { return (m_left == nullptr && m_right == nullptr); }
    inline const AABB& bbox() const { return m_bbox; }

    void traverse(const Ray& ray, TriangleVisitor& visitor) const; // calls visitor.visit


private:
    BVH(const std::vector<std::shared_ptr<Mesh>>& meshes,
        std::vector<std::pair<size_t,size_t>>& indexPairSet,
        size_t begin,
        size_t end);

    std::vector<std::pair<size_t,size_t>> makeIndexPairSet(const std::vector<std::shared_ptr<Mesh>>& meshes);


    AABB m_bbox;
    BVH* m_left = nullptr;
    BVH* m_right = nullptr;
    size_t m_meshIndex = size_t(-1);
    size_t m_triangleIndex = size_t(-1);
};
