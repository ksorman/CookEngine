#include "Camera.h"


namespace CookEngine {

Camera::Camera()
{
    m_rotation = glm::quatLookAt(glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
    m_aspectRatio = 1.0f;
    m_FOV = 45.0f;
    m_position = glm::vec3(0.0f, 0.0f, -2.0f);
    UpdateMatrixs();
}

void Camera::SetAspectRatio(float aspectRatio)
{
    m_aspectRatio = aspectRatio;
    UpdateProj();
}

void Camera::UpdateProj()
{
    m_proj = glm::perspective(glm::radians(m_FOV), m_aspectRatio, m_zNear, m_zFar);
    m_proj[1][1] *= -1;// flip y
    m_invProj = glm::inverse(m_proj);
}

void Camera::UpdateView()
{
    glm::mat4 rotate = glm::transpose(glm::mat4_cast(m_rotation));
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), -m_position);
    m_view = rotate * translate;
    m_invView = glm::inverse(m_view);
}

void Camera::UpdateMatrixs()
{
    UpdateProj();
    UpdateView();
}

glm::mat4 Camera::GetProj() const
{
    return m_proj;
}

glm::mat4 Camera::GetView() const
{
    return m_view;
}

void Camera::SetFOV(float FOV)
{
    m_FOV = FOV;
    UpdateProj();
}

glm::vec3 Camera::GetPosition() const
{
    return m_position;
}

void Camera::SetPosition(const glm::vec3& position)
{
    m_position = position;
    UpdateView();
}
}// namespace CookEngine