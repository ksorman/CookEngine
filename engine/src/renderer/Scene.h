#ifndef SCENE_H
#define SCENE_H

#include "Camera.h"
#include "entt/entity/fwd.hpp"
#include "utils/Model.h"
#include <entt/entt.hpp>
#include <memory>

namespace CookEngine {
class Scene
{
  public:
    Scene();
    Camera& GetCamera();
    const Camera& GetCamera() const;

    void AddModel(std::shared_ptr<Model> model);

    entt::registry& GetRegistry();
  private:
    entt::registry m_registry;
    Camera m_camera;
};
}// namespace CookEngine

#endif
