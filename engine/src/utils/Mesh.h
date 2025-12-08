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

    VkBuffer vertexBuffer;
    VmaAllocation vertexBufferMemory;

    VkBuffer indexBuffer;
    VmaAllocation indexBufferMemory;
};

}// namespace CookEngine

#endif