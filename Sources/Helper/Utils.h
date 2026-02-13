#pragma once

#include "../Core/Mesh.h"
#include "../RayHit/Ray.h"

#include <glm/glm.hpp>
#include <random>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>


// Simple utility functions
class Utils {

public:
	// -------- Intersection Tests --------
	static bool intersectTriangle(const Ray& ray,const std::shared_ptr<Mesh> mesh,const glm::uvec3& tri,Hit& hit)
	{
		const auto& P = mesh->vertexPositions();
		glm::mat4 M = mesh->computeTransformMatrix();

		glm::vec3 v0 = glm::vec3(M * glm::vec4(P[tri[0]], 1.0f));
		glm::vec3 v1 = glm::vec3(M * glm::vec4(P[tri[1]], 1.0f));
		glm::vec3 v2 = glm::vec3(M * glm::vec4(P[tri[2]], 1.0f));

		// Möller–Trumbore intersection algorithm
		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;

		glm::vec3 h = glm::cross(ray.direction, edge2);
		float a = glm::dot(edge1, h);

		// ray is parallel to triangle 
		if (std::abs(a) < 1e-8f) return false;

		// intersection lies outside the triangle
		float f = 1.0f / a;
		glm::vec3 s = ray.origin - v0;
		float u = f * glm::dot(s, h);
		if (u < 0.0f || u > 1.0f) return false;

		// intersection lies outside the triangle
		glm::vec3 q = glm::cross(s, edge1);
		float v = f * glm::dot(ray.direction, q);
		if (v < 0.0f || u + v > 1.0f) return false;

		float t = f * glm::dot(edge2, q);
		if (t <= 1e-6f) return false;

		
		// closest hit 
		hit.t = t;
		hit.position = ray.origin + t * ray.direction;
		hit.color = mesh->color();

		// compute normal with barycentric coord for smooth shading
		float w = 1.0f - u - v;
		glm::vec3 n0 = mesh->vertexNormals()[tri[0]];
		glm::vec3 n1 = mesh->vertexNormals()[tri[1]];
		glm::vec3 n2 = mesh->vertexNormals()[tri[2]];

		glm::mat3 Nmat = glm::transpose(glm::inverse(glm::mat3(M)));
		n0 = glm::normalize(Nmat * n0);
		n1 = glm::normalize(Nmat * n1);
		n2 = glm::normalize(Nmat * n2);

		hit.normal = glm::normalize(w * n0 + u * n1 + v * n2);

		return true;
	}

	// --- Uniform sampling (not used anymore) ---
    static glm::vec2 sampleDisk()
    {
		// thread-local random number generator
		// https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution.html

		std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		float r = std::sqrt(dist(rng)); // radius in [0,1], sqrt for uniform distribution
		float theta = dist(rng) * glm::pi<float>() * 2.0f;  // angle in [0, 2pi]

		// return cartesian coordinates (x,y)
		float x = r * std::cos(theta);
		float y = r * std::sin(theta);

		return glm::vec2(x,y);
    }

	// -- Stratified sampling (not used anymore) --
	static glm::vec2 sampleDiskStratified(int s, int numSamples)
	{
		// determine grid size and cell indices
		int k = int(std::sqrt(numSamples));
		int i = s % k; // sample's grid cell x index
		int j = s / k; // sample's grid cell y index

		static thread_local std::mt19937 rng(1337);
		static thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);

		// sample within the cell
		float u = (i + dist(rng)) / k; 
		float v = (j + dist(rng)) / k;

		float r = std::sqrt(u);				    	// radius in [0,1], sqrt for uniform distribution
		float theta = 2.f * glm::pi<float>() * v;   // angle in [0, 2pi]

		// return cartesian coordinates (x,y)
		float x = r * std::cos(theta);
		float y = r * std::sin(theta);

		return glm::vec2(x,y);
	}

	// ---- Poisson sampling (not used anymore) ----
	static glm::vec2 samplePoissonDisk()
	{
		static thread_local std::mt19937 rng(1337);
		static thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);

		float r = std::sqrt(dist(rng));
		float theta = 2.f * glm::pi<float>() * dist(rng);

		// return cartesian coordinates (x,y)
		float x = r * std::cos(theta);
		float y = r * std::sin(theta);

		return glm::vec2(x,y);
	}

	// --------- Hammersley sampling ----------
	// (code taken from: https://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html)
	static float radicalInverse(uint32_t bits) {
		// Compute the radical inverse of a 32-bit integer using Van der Corput sequence
		bits = (bits << 16u) | (bits >> 16u);
		bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
		bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
		bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
		bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
		return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
	}

	// Hammersley point set generation
	static glm::vec2 hammersley(uint32_t i, uint32_t N)
	{
		return { float(i) / float(N), radicalInverse(i) };
	}

	// Convert HSV color to RGB
	static glm::vec3 HSVtoRGB(float h, float s, float v) {
		//(h in[0, 1] or degrees, s in[0, 1] or [0, 255], v in[0, 1])
		// returns RGB in [0,1]
		// code taken from previous lab, adapted from python


		// support s in [0,255]
		if (s > 1.0f) s = s / 255.0f;

		// convert h to degrees
		h *= 360.0f;

		// clamp inputs (h can wrap around, s and v clamped to [0,1])
		h = std::fmod(h, 360.0f);
		if (h < 0.0f) h += 360.0f;
		s = std::clamp(s, 0.0f, 1.0f);
		v = std::clamp(v, 0.0f, 1.0f);

		float c = v * s;
		float h_div60 = h / 60.0f;
		float x = c * (1.0f - std::fabs(std::fmod(h_div60, 2.0f) - 1.0f));
		float m = v - c; // match value to get final RGB in [0,1]

		float rp = 0.0f, gp = 0.0f, bp = 0.0f; // intermediate RGB values before adding m
		int sector = static_cast<int>(std::floor(h_div60)) % 6;
		switch (sector) {
			case 0: rp = c; gp = x; bp = 0; break;
			case 1: rp = x; gp = c; bp = 0; break;
			case 2: rp = 0; gp = c; bp = x; break;
			case 3: rp = 0; gp = x; bp = c; break;
			case 4: rp = x; gp = 0; bp = c; break;
			case 5: rp = c; gp = 0; bp = x; break;
			default: rp = 0; gp = 0; bp = 0; break;
		}

		return glm::vec3(rp + m, gp + m, bp + m);
	}

	static void printHelp() {
		std::cout << "\n======== Controls ==========\n";
		std::cout << "H                 : Help\n";
		std::cout << "ESC               : Quit\n";
		std::cout << "Mouse             : Orbit camera\n";
		std::cout << "G                 : Toggle Gizmo\n";
		std::cout << "F                 : Toggle Depth of Field rendering\n";
		std::cout << "Shift+RightClick  : Adjust Focus Plane\n";
		std::cout << "===============================\n\n";
		return;
	}

};