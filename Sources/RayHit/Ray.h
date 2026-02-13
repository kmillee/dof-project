#pragma once

#include <glm/glm.hpp>

// Ray as they are traced through the scene (before lens refraction)
struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};

// Struct to hold detailed info about a ray passing through the lens (used for RaySim)
struct LensRay
{
    glm::vec3 hitEnter;
    glm::vec3 hitExit;
    glm::vec3 dirInside;
    glm::vec3 dirOutside;
};