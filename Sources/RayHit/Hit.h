#pragma once
#include <glm/glm.hpp>


struct Hit {
	float t;            // distance before intersection
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    int meshID;
};

// Stores multiple hits per ray (2 are enough)
struct RayHits {
    int count = 0;
    static constexpr int MAX_HITS = 2;
    Hit hits[MAX_HITS];
};


