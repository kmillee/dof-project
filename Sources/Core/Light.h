#pragma once

#include <glm/glm.hpp>

class Light
{
public:
    glm::vec3 position = { 2.f, 3.f, 2.f };
    glm::vec3 color = { 1.f, 1.f, 1.f };

    float intensity = 1.0f;
    float shininess = 64.0f;

    Light() = default; 
};
