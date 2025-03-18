#ifndef SCENE_H
#define SCENE_H

#include "Camera.h"
#include "entt/entity/fwd.hpp"
#include <entt/entt.hpp>

namespace CookEngine {
class Scene
{
  public:
    Scene();
    Camera& GetCamera();
    const Camera& GetCamera() const;
  private:
    entt::registry m_registry;
    Camera m_camera;
};
}// namespace CookEngine

#endif
