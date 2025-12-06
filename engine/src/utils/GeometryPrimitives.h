#ifndef GEOMETRY_PRIMITIVES_H
#define GEOMETRY_PRIMITIVES_H

#include "MathUsage.h"

namespace CookEngine {
struct Vertex
{
    glm::vec3 position;
    glm::vec2 tc;

    void Reset()
    {
        position = glm::vec3(0.0f);
        tc = glm::vec2(0.0f);
    }
};
}// namespace CookEngine

#endif