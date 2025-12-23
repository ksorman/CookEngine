#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include "VmaUsage.h"

namespace CookEngine {

class CommandPool
{
  public:
    CommandPool(VkDevice device, uint32_t quequeIndex);
    ~CommandPool();

    CommandPool(CommandPool&) = delete;
    CommandPool(CommandPool&&) = delete;
    CommandPool operator=(CommandPool&) = delete;
    CommandPool operator=(CommandPool&&) = delete;

    operator VkCommandPool&();

  private:
    VkCommandPool m_commandPool;
    VkDevice m_device;
};


}// namespace CookEngine

#endif