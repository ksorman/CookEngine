#ifndef VULKAN_ALLOCATOR_H
#define VULKAN_ALLOCATOR_H

#include "VmaUsage.h"

namespace CookEngine {
    class VulkanAllocator{
        public:
        VulkanAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance);
        ~VulkanAllocator();
        operator VmaAllocator();
        private:
            VmaAllocator m_allocator;
    };
}

#endif