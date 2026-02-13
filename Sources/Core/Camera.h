#pragma once

#include "../RayHit/Ray.h"

#include <glm/glm.hpp>

class Camera {
public:
    Camera();

	// ---- Ray generation ----
    Ray pinholeRay(int x, int y, int W, int H) const;
	Ray thinLensRay(Ray pinhole, int s, int numSamples) const;

	// ---- Camera controls -----
	void zoom(float dt);
	void orbit(float dx, float dy);
    void translate(float dx, float dy);
    void reset();

	// ---- Camera parameters ----
    float fov = 60.f;   
    float nearPlane = 0.1f;
    float farPlane = 500.f;

	// ---- Depth of Field parameters ----
    float focusDistance = 10.0f;
    float aperture = 0.005f; 

	
	// ----- Getters -----
	glm::vec3 getForward() const { return forward; }
	glm::vec3 getRight() const { return right; }
	glm::vec3 getUp() const { return up; }
	glm::vec3 getPosition() const;
	glm::mat4 viewMatrix() const;
	glm::mat4 projectionMatrix(int w, int h) const;

	float& getAperture() { return aperture; }
	float& getFocusDistance() { return focusDistance; }

	// ---- Setters -----
	void setPosition(const glm::vec3& position) {
		pos = position;
		updateBasis();
	}
	void setAperture(float a) {
		aperture = a;
	}

private: 
	void updateBasis(); // update forward, right, up vectors

	// Spherical coordinates
    glm::vec3 target = { -2.2f,2.2f,0 };
    float hAngle = 0.0f;
    float vAngle = 0.0f;
    float distance = 15.0f;

	// Basis vectors
	glm::vec3 pos;      
	glm::vec3 forward;  // = normalize(target - pos) i.e. lookAt direction
	glm::vec3 right; // = cross(forward, up)
	glm::vec3 up;    // = cross(right, forward)
};
