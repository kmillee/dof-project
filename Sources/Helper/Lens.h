#pragma once

#include "../RayHit/Ray.h"

#include <glm/glm.hpp>
#include <optional> 

class Lens
{
public:
	Lens() = default;

	// Lens parameters, biconvex lens (thin)
	float R1 = 30.0f;       // curv radius (convex, light -> glass i.e. positive)
	float R2 = -30.0f;      // curv radius (convex, glass -> light i.e. negative)
	float thickness = 5.0f; // in mm
	float ior = 1.5f;       // indice of refraction (glass)

	glm::vec3 position = { 0, 0, 0 };  // lens center position
    glm::vec3 forward = { 0, 0, -1 }; 

	// Complete refraction through the lens (refract x2)
    std::optional<Ray> refract(const Ray& inRay) const;
    std::optional<LensRay> trace(const Ray& ray);

private:
    bool intersectSphere(
        const Ray& ray,
        const glm::vec3& center,
        float radius,
        float& t) const;

    bool refractRay(
        const glm::vec3& I,
        const glm::vec3& N,
        float eta,
        glm::vec3& T) const;
};