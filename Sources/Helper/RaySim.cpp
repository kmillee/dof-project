#pragma once

#include "RaySim.h"

// Generate a grid of parallel rays
void RaySim::generateParallelRays(int N, float size)
{
	m_rays.clear();
	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			float x = (static_cast<float>(i) / (N - 1) - 0.5f) * size;
			float y = (static_cast<float>(j) / (N - 1) - 0.5f) * size;
			Ray ray;
			ray.origin = glm::vec3(x, y, 100.0f); // start far away on z+
			ray.direction = glm::vec3(0, 0, -1);   // pointing towards -z
			m_rays.push_back(ray);
		}
	}
}

// Update rays by tracing them through the lens
void RaySim::update(Lens& lens)
{
    m_debugLines.clear();

    for (const auto& ray : m_rays)
    {
		// check if the ray hits the lens
		auto refractedRayOpt = lens.trace(ray);
		if (!refractedRayOpt)
			continue;

		const LensRay& lensRay = *refractedRayOpt;

		// Incoming ray (blue)
		m_debugLines.push_back({
			ray.origin,
			lensRay.hitEnter,
			glm::vec3(0, 0, 1) // blue
			});
		
		// Inside lens ray (green)
		m_debugLines.push_back({
			lensRay.hitEnter,
			lensRay.hitExit,
			glm::vec3(0, 1, 0) // green
			});


		// Outgoing ray (red)
        m_debugLines.push_back({
            lensRay.hitExit,
            lensRay.hitExit + lensRay.dirOutside * 100.0f,
			glm::vec3(1, 0, 0) // red
            });
    }

	// optocal axis (yellow)
    m_debugLines.push_back({
		glm::vec3(lens.position.x, lens.position.y, 100.0f),
		glm::vec3(lens.position.x, lens.position.y, -100.0f),
		glm::vec3(1, 1, 0) // yellow
        });
}


void RaySim::draw(DebugLine& debug)
{
	debug.clear();
	for (const auto& line : m_debugLines)
	{
		debug.addLine(line.a, line.b, line.color);
	}
}