#pragma once

#include "HitCollector.h"
#include "../Helper/Utils.h"
#include "../Core/Mesh.h"

HitCollector::HitCollector(const Ray& r,
    const std::vector<std::shared_ptr<Mesh>>& meshes,
    RayHits& hits)
    : m_ray(r), m_meshes(meshes), m_hits(hits)
{
}

void HitCollector::visit(size_t meshID, size_t triID) {
    const auto& mesh = m_meshes[meshID];
    const auto& tri = mesh->triangleIndices()[triID];

    Hit h;
    if (!Utils::intersectTriangle(m_ray, mesh, tri, h))
        return;


    h.meshID = static_cast<int>(meshID);
    h.color = mesh->color();


    // Sorts hits by distance
    int i = m_hits.count;
    if (i < RayHits::MAX_HITS) {
        m_hits.hits[i] = h;
        m_hits.count++;
    }
    else if (h.t >= m_hits.hits[RayHits::MAX_HITS - 1].t) {
        return; 
    }
    else {
        i = RayHits::MAX_HITS - 1;
        m_hits.hits[i] = h;
    }
    
    // bubble up
    while (i > 0 && m_hits.hits[i].t < m_hits.hits[i - 1].t) {
        std::swap(m_hits.hits[i], m_hits.hits[i - 1]);
        --i;
    }
}
