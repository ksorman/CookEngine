#ifndef MESH_H
#define MESH_H

#include <cstdint>
#include <vector>
#include "GeometryPrimitives.h"

namespace CookEngine {
struct Mesh
{
    std::vector<Vertex> verteces;
    std::vector<uint32_t> indeces;
};

}// namespace CookEngine

#endif