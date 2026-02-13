#pragma once

#include "Ray.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <limits>

// Axis-aligned bounding box in object (local) space
// Taken from a previous class
struct AABB {
    glm::vec3 min{ std::numeric_limits<float>::infinity() };
    glm::vec3 max{ -std::numeric_limits<float>::infinity() };

    // Ray must be in the same space as the AABB (local/object space)
    bool intersect(const Ray& r) const;
};

inline bool AABB::intersect(const Ray& r) const {
    // Use inverse-direction slab method, handle zero components safely
    glm::vec3 invDir;
    invDir.x = (r.direction.x != 0.0f) ? 1.0f / r.direction.x : std::numeric_limits<float>::infinity();
    invDir.y = (r.direction.y != 0.0f) ? 1.0f / r.direction.y : std::numeric_limits<float>::infinity();
    invDir.z = (r.direction.z != 0.0f) ? 1.0f / r.direction.z : std::numeric_limits<float>::infinity();

    float t1 = (min.x - r.origin.x) * invDir.x;
    float t2 = (max.x - r.origin.x) * invDir.x;
    float tmin = std::min(t1, t2);
    float tmax = std::max(t1, t2);

    t1 = (min.y - r.origin.y) * invDir.y;
    t2 = (max.y - r.origin.y) * invDir.y;
    float tymin = std::min(t1, t2);
    float tymax = std::max(t1, t2);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    tmin = std::max(tmin, tymin);
    tmax = std::min(tmax, tymax);

    t1 = (min.z - r.origin.z) * invDir.z;
    t2 = (max.z - r.origin.z) * invDir.z;
    float tzmin = std::min(t1, t2);
    float tzmax = std::max(t1, t2);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    tmin = std::max(tmin, tzmin);
    tmax = std::min(tmax, tzmax);

    // Return true if intersection interval overlaps ray (tmax >= max(tmin, 0))
    return tmax >= std::max(tmin, 0.0f);
}
