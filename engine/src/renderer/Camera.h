#ifndef CAMERA_H
#define CAMERA_H

#include "utils/UsingMath.h"

namespace CookEngine {
class Camera
{
  public:
    Camera();
    void SetAspectRatio(float aspectRatio);
    void UpdateProj();
    void UpdateView();
    void UpdateMatrixs();
    glm::mat4 GetProj() const;
    glm::mat4 GetView() const;
    void SetFOV(float FOV);

    glm::vec3 GetPosition() const;
    void SetPosition(const glm::vec3& position);
  private:
    float m_aspectRatio;

    float m_FOV;
    glm::vec3 m_position;
    glm::quat m_rotation;
    float m_zNear = 0.1f;
    float m_zFar = 10.0f;

    glm::mat4 m_view;
    glm::mat4 m_proj;

    glm::mat4 m_invView;
    glm::mat4 m_invProj;
};
}// namespace CookEngine

#endif// CAMERA_H
