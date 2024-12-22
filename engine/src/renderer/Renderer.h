#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>

namespace CookEngine {
class Renderer {
public:
  void Init();
  void Deinit();
private:
    void CreateInstance(VkInstance* instance);
    void DestrouyInstance(VkInstance instance);
  VkInstance m_vkInstance;
};
} // namespace CookEngine
#endif // RENDERER_H