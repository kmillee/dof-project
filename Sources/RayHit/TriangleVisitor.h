#pragma once

// Interface for visiting triangles during ray traversal (ray hit, depth peeling...)
// (see HitCollector)
class TriangleVisitor {
public:
    virtual ~TriangleVisitor() {}
    virtual void visit(size_t meshID, size_t triID) = 0; // called when triangle is intersected
};
