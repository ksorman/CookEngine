#ifndef GEOMETRY_PRIMITIVES_H
#define GEOMETRY_PRIMITIVES_H

#include "MathUsage.h"
#include "renderer/VmaUsage.h"
#include <array>
#include <cstddef>
namespace CookEngine {
struct Vertex
{
    glm::vec3 position;
    glm::vec2 tc;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription vertexDescription{};
        vertexDescription.binding = 0;
        vertexDescription.stride = sizeof(Vertex);
        vertexDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return vertexDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> vertexAtributeDescription{};

        // position
        vertexAtributeDescription[0].binding = 0;
        vertexAtributeDescription[0].location = 0;
        vertexAtributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexAtributeDescription[0].offset = offsetof(Vertex, position);

        // texture
        vertexAtributeDescription[1].binding = 0;
        vertexAtributeDescription[1].location = 1;
        vertexAtributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
        vertexAtributeDescription[1].offset = offsetof(Vertex, tc);

        return vertexAtributeDescription;
    }

    void Reset()
    {
        position = glm::vec3(0.0f);
        tc = glm::vec2(0.0f);
    }
};
}// namespace CookEngine

#endif