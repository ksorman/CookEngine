#include "Camera.h"
#include "glm/ext/quaternion_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"

#define FIXED_UP_VECTOR 1

namespace CookEngine {

Camera::Camera()
{
    m_rotation = glm::quatLookAt(glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)), UP);
    m_rotationEulers = glm::eulerAngles(m_rotation);
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
#if FIXED_UP_VECTOR
    glm::quat rotationTmp = glm::rotate(m_rotation, m_rotationEulers.y, UP);
    rotationTmp = glm::rotate(rotationTmp, m_rotationEulers.x, RIGTH);
    glm::mat4 rotate = glm::transpose(glm::mat4_cast(rotationTmp));
#else
    glm::mat4 rotate = glm::transpose(glm::mat4_cast(m_rotation));
#endif
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), -m_position);
    m_view = rotate * translate;
    m_invView = glm::inverse(m_view);
    m_right = m_view[0];
    m_up = m_view[1];
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

void Camera::MoveReletive(const glm::vec3& offset)
{
    glm::vec3 offsetWorld = m_invView * glm::vec4(offset, 0.0f);
    m_position += offsetWorld;
    UpdateView();
}

void Camera::RotateReletive(const glm::vec3& angels)
{
    m_rotationEulers += angels;
#if FIXED_UP_VECTOR
#else
    m_rotation = glm::rotate(m_rotation, angels.x, m_right);
    m_rotation = glm::rotate(m_rotation, angels.y, m_up);
#endif
    UpdateView();
}
}// namespace CookEngine