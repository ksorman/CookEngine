#include "Instance.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

namespace CookEngine {

Instance::Instance()
{
    if (volkInitialize() != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to initialize Volk!");
    }

    VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Demo",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "CookEngine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0 };

    std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo insatnceCreateInfo{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions };

    auto vkResult = vkCreateInstance(&insatnceCreateInfo, nullptr, &m_instance);
    if (vkResult == VK_SUCCESS) {
        volkLoadInstance(m_instance);
        spdlog::info("[Vulkan] Instance was created successfully");
    } else {
        spdlog::error("[Vulkan] Instance wasn't created");
    }
}

Instance::~Instance()
{
    vkDestroyInstance(m_instance, nullptr);
}

Instance::operator VkInstance()
{
    return m_instance;
}

}// namespace CookEngine