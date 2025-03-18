#include "Scene.h"
#include "Camera.h"

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
}// namespace CookEngine