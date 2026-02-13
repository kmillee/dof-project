#pragma once

#include "TriangleVisitor.h"
#include "Ray.h"
#include "../Core/Mesh.h"

#include <vector>
#include <memory>

class HitCollector : public TriangleVisitor {
public:
    HitCollector(const Ray& r,
                 const std::vector<std::shared_ptr<Mesh>>& meshes,
                 RayHits& hits);

    void visit(size_t meshID, size_t triID) override;

private:
    const Ray& m_ray;
    const std::vector<std::shared_ptr<Mesh>>& m_meshes;
    RayHits& m_hits;
};