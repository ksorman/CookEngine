#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
namespace CookEngine {

struct Model
{
    std::vector<Mesh> meshes;
};

}// namespace CookEngine
#endif