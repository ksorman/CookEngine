#ifndef INSTANCE_H
#define INSTANCE_H

#include "VmaUsage.h"

namespace CookEngine {

class Instance
{
  public:
    Instance();
    ~Instance();

    Instance(Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance operator=(Instance&) = delete;
    Instance operator=(Instance&&) = delete;

    operator VkInstance();

  private:
    VkInstance m_instance;
};
}// namespace CookEngine

#endif