#pragma once

#include "Lens.h"

#include <glm/gtc/constants.hpp>
#include <cmath>


// Lens class used to visualize refracted rays in scene 
// (not used in project pipeline, used for visualization early on)

// Taken from previous lab
// (help from https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection)
bool Lens::intersectSphere(const Ray& ray,const glm::vec3& center,float radius,float& t) const
{
    glm::vec3 oc = ray.origin - center;

    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - radius * radius;

    float disc = b * b - 4 * a * c;
    if (disc < 0.0f)
        return false;

    float sqrtD = std::sqrt(disc);
    float t0 = (-b - sqrtD) / (2 * a);
    float t1 = (-b + sqrtD) / (2 * a);

    t = (t0 > 0) ? t0 : t1;
    return t > 0;
}

// Snell-Descartes refraction
// see https://en.wikipedia.org/wiki/Snell%27s_law#Vector_form
bool Lens::refractRay(const glm::vec3& I,const glm::vec3& N,float eta,glm::vec3& T) const
{
    float cosi = glm::clamp(glm::dot(-I, N), -1.0f, 1.0f);
    float k = 1.0f - eta * eta * (1.0f - cosi * cosi);

    if (k < 0.0f) return false;

    T = eta * I + (eta * cosi - std::sqrt(k)) * N;
    return true;
}

// Full lens refraction
std::optional<Ray> Lens::refract(const Ray& inRay) const
{
	// Centers of the spherical surfaces
    glm::vec3 C1 = position + forward * R1;
    glm::vec3 C2 = position + forward * (thickness + R2);

    // ---------- Surface 1 ----------
	// From air to glass
    float t1;
    if (!intersectSphere(inRay, C1, std::abs(R1), t1))
        return std::nullopt;

    glm::vec3 P1 = inRay.origin + t1 * inRay.direction;
    glm::vec3 N1 = glm::normalize(P1 - C1);
    if (R1 < 0) N1 = -N1;

    glm::vec3 D1;
    if (!refractRay(inRay.direction, N1, 1.0f / ior, D1))
        return std::nullopt;

    Ray insideRay{ P1 + D1 * 1e-4f, glm::normalize(D1) };

    // ---------- Surface 2 ----------
	// From glass to air
    float t2;
    if (!intersectSphere(insideRay, C2, std::abs(R2), t2))
        return std::nullopt;

    glm::vec3 P2 = insideRay.origin + t2 * insideRay.direction;
    glm::vec3 N2 = glm::normalize(P2 - C2);
    if (R2 > 0) N2 = -N2;

    glm::vec3 D2;
    if (!refractRay(insideRay.direction, N2, ior, D2))
        return std::nullopt;

    return Ray{ P2 + D2 * 1e-4f, glm::normalize(D2) };
}

// Trace ray through the lens, returning detailed info on entry/exit points and directions
std::optional<LensRay> Lens::trace(const Ray& inRay)
{
    LensRay out;

    // === Surface centers ===
    glm::vec3 C1 = position + forward * R1;
    glm::vec3 C2 = position + forward * (thickness + R2);

    // ============================================================
    // Surface 1: air -> glass
    // ============================================================
    float t1;
    if (!intersectSphere(inRay, C1, std::abs(R1), t1))
        return std::nullopt;

    out.hitEnter = inRay.origin + t1 * inRay.direction;

    glm::vec3 N1 = glm::normalize(out.hitEnter - C1);
    if (R1 < 0.0f) N1 = -N1;

    if (!refractRay(inRay.direction, N1, 1.0f / ior, out.dirInside))
        return std::nullopt;

    out.dirInside = glm::normalize(out.dirInside);

    // Ray inside lens
    Ray insideRay;
    insideRay.origin = out.hitEnter + out.dirInside * 1e-4f;
    insideRay.direction = out.dirInside;


    // ============================================================
    // Surface 2: glass -> air
    // ============================================================
    float t2;
    if (!intersectSphere(insideRay, C2, std::abs(R2), t2))
        return std::nullopt;

    out.hitExit = insideRay.origin + t2 * insideRay.direction;

    glm::vec3 N2 = glm::normalize(out.hitExit - C2);

	// check normal orientation (should point against the ray direction)
    if (glm::dot(N2, insideRay.direction) > 0.0f)
        N2 = -N2;

    if (!refractRay(insideRay.direction, N2, ior, out.dirOutside))
        return std::nullopt;

    out.dirOutside = glm::normalize(out.dirOutside);

    return out;
}
