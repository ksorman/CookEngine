#include "Scene.h"
#include "Camera.h"
#include <memory>

namespace CookEngine {
Scene::Scene() {}

Camera& Scene::GetCamera()
{
    return m_camera;
}

const Camera& Scene::GetCamera() const
{
    return m_camera;
}

void Scene::AddModel(std::shared_ptr<Model> model)
{
    const auto entity = m_registry.create();
    m_registry.emplace<std::shared_ptr<Model>>(entity, std::move(model));
}

entt::registry& Scene::GetRegistry()
{
    return m_registry;
}
}// namespace CookEngine