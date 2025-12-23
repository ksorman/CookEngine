#include "Pipeline.h"
#include "spdlog/spdlog.h"

namespace CookEngine {

Pipeline::~Pipeline()
{
    DestroyPipeline();
}

bool Pipeline::CreatePipeline(VkDevice device, const VkGraphicsPipelineCreateInfo* createInfo)
{
    m_device = device;

    if (!createInfo) {
        spdlog::error("[Vulkan] Failed to create graphics pipeline! Create Info is Null!");
        return false;
    }

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, createInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create graphics pipeline!");
        return false;
    }

    spdlog::info("[Vulkan] Graphics pipeline created successfully!");
    m_isInitialized = true;
    return true;
}

void Pipeline::DestroyPipeline()
{
    if (m_isInitialized) {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        m_isInitialized = false;
    }
}

Pipeline::operator VkPipeline&()
{
    return m_pipeline;
}

}// namespace CookEngine