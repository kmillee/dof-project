#pragma once

#include "Camera.h"
#include "../Helper/Utils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <iostream>


Camera::Camera()
{
    target = { -1.7f,2.2f,0 };
    distance = 3.0f;
    hAngle = glm::radians(0.f);
    vAngle = glm::radians(0.f);
	updateBasis(); // initialize basis vectors
}

// Update camera basis vectors (forward, right, up)
void Camera::updateBasis()
{
	pos = getPosition();
	forward = glm::normalize(target - pos); // lookAt direction
	right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
	up = glm::cross(right, forward);   
}

// Update camera position based on spherical coordinates
glm::vec3 Camera::getPosition() const
{
    return {
        target.x + distance * cos(vAngle) * sin(hAngle),
        target.y + distance * sin(vAngle),
        target.z + distance * cos(vAngle) * cos(hAngle)
    };
}

void Camera::orbit(float dx, float dy)
{
    hAngle += dx * 0.001f;
    vAngle += dy * 0.001f;

    float limit = glm::radians(89.0f);
    vAngle = glm::clamp(vAngle, -limit, limit);

    updateBasis();
}

void Camera::zoom(float delta)
{
    distance *= (1.0f - delta * 0.1f);
    distance = glm::clamp(distance, 0.5f, 50.0f);
    updateBasis();
}

void Camera::translate(float dx, float dy)
{
    float speed = distance * 0.001f;
    glm::vec3 offset = (-right * dx + up * dy) * speed;
    target += offset;

    updateBasis();
}


glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(getPosition(), target, {0,1,0});
}

glm::mat4 Camera::projectionMatrix(int w, int h) const
{
    float aspect = float(w) / float(h);
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

// Generate a pinhole camera ray through pixel (i, j)
Ray Camera::pinholeRay(int i, int j, int W, int H) const
{
    float aspect = float(W) / float(H);
    float tanHalfFov = tan(glm::radians(fov) * 0.5f);

    float u = (i + 0.5f) / float(W);
    float v = (j + 0.5f) / float(H);

    float x = (2.0f * u - 1.0f) * aspect * tanHalfFov;
    float y = (1.0f - 2.0f * v) * tanHalfFov;

    glm::vec3 dir = glm::normalize(forward + x * right + y * up);

    return Ray{ pos, dir };
}

// Generate sample point on unit disk using concentric mapping
Ray Camera::thinLensRay(Ray pinhole, int sample_iter, int numSamples) const
{
    // This point is in focus for the pixel
    glm::vec3 focalPoint = pos + pinhole.direction * focusDistance;

	// sample point on unit disk (uniform)
	//glm::vec2 sample = Utils::sampleDisk();

    // sample point (stratified)
	//int local_iter = sample_iter % numSamples;
	//glm::vec2 sample = Utils::sampleDiskStratified(local_iter, numSamples);
    
    // sample point (Poisson)
    //glm::vec2 sample = Utils::samplePoissonDisk();
     
    // sample point (Hammersley)
    glm::vec2 sample = Utils::hammersley(sample_iter, numSamples);

	// scale to aperture size
    glm::vec3 lensPoint = pos + sample.x * right * aperture + sample.y * up * aperture;

    // independent from sample, every ray from pixel p will go through focalPoint
    glm::vec3 dir = glm::normalize(focalPoint - lensPoint);

    return Ray{ lensPoint, dir };
}
