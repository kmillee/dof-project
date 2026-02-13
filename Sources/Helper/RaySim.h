#pragma once

#include "Lens.h"
#include "DebugLine.h"

#include <vector>
#include <glm/glm.hpp>

// Class to visualize parallel rays and how they refract in a lens
class RaySim
{
public:
	RaySim() = default;
    void generateParallelRays(int N, float size);
    void update(Lens& lens);
    void draw(DebugLine& debug);

private:
    std::vector<Ray> m_rays;
    std::vector<Line> m_debugLines;
};

