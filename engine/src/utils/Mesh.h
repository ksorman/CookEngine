#ifndef MESH_H
#define MESH_H

#include <cstdint>
#include <memory>
#include <vector>
#include "GeometryPrimitives.h"
#include "renderer/GPUBuffer.h"

namespace CookEngine {
struct Mesh
{
    std::vector<Vertex> verteces;
    std::vector<uint32_t> indeces;

    bool renderInitilized = false;

    std::unique_ptr<GPUBuffer> vertexBuffer;
    std::unique_ptr<GPUBuffer> indexBuffer;
};

}// namespace CookEngine

#endif