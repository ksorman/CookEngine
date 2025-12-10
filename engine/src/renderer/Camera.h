#ifndef CAMERA_H
#define CAMERA_H

#include "utils/MathUsage.h"

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

    void MoveReletive(const glm::vec3& offset);
    void RotateReletive(const glm::vec3& angels);
  private:
    const glm::vec3 UP = {0.0f, 1.0f, 0.0f};
    const glm::vec3 RIGTH = {1.0f, 0.0f, 0.0f};

    float m_aspectRatio;

    float m_FOV;
    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::vec3 m_rotationEulers;
    
    float m_zNear = 0.1f;
    float m_zFar = 1000.0f;

    glm::mat4 m_view;
    glm::mat4 m_proj;

    glm::mat4 m_invView;
    glm::mat4 m_invProj;

    glm::vec3 m_right;
    glm::vec3 m_up;
};
}// namespace CookEngine

#endif// CAMERA_H
