#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include "VmaUsage.h"

namespace CookEngine {

class CommandPool
{
  public:
    CommandPool(VkDevice& device, uint32_t quequeIndex);
    ~CommandPool();
    operator VkCommandPool();
  private:
    VkCommandPool m_commandPool;
    VkDevice& m_device;
};


}// namespace CookEngine

#endif