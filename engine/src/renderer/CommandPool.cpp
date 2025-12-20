#include "CommandPool.h"
#include "spdlog/spdlog.h"

namespace CookEngine {
    CommandPool::CommandPool(VkDevice& device, uint32_t quequeIndex) : m_device(device)
    {
        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = quequeIndex;

        if(vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool)){
            spdlog::info("[Vulkan] Command pool created successfully");
        }
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        spdlog::debug("[Vulkan] Command pool destroyed successfully");
    }

    CommandPool::operator VkCommandPool()
    {
        return m_commandPool;
    }
}