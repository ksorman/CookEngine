#include "Shader.h"

#include <Windows.h>
#include <atlbase.h>
#include <dxcapi.h>
#include <string_view>

namespace CookEngine {

Shader::Shader(VkDevice device, std::wstring_view fileName, ShaderType type, void* shaderBlobPtr, size_t shaderBlobSize)
  : m_device(device), m_fileName(fileName), m_type(type)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderBlobSize;
    shaderModuleCreateInfo.pCode = static_cast<uint32_t*>(shaderBlobPtr);

    vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &m_shaderModule);
}

Shader::~Shader()
{
    if (m_shaderModule != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
    }
}

VkShaderModule Shader::GetShaderModul() const
{
    return m_shaderModule;
}

Shader::operator VkShaderModule() const
{
    return m_shaderModule;
}

VkPipelineShaderStageCreateInfo Shader::GetPiplineShaderStageCreateInfo() const
{
    VkShaderStageFlagBits stage;
    switch (m_type) {

    case ShaderType::VERTEX:
        stage = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case ShaderType::FRAGMENT:
        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    case ShaderType::UNKNOWN:
    default:
        break;
    }

    VkPipelineShaderStageCreateInfo shaderStageInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = stage,
        .module = m_shaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr };
    return shaderStageInfo;
}

Shader::operator VkPipelineShaderStageCreateInfo() const
{
    return GetPiplineShaderStageCreateInfo();
}
}// namespace CookEngine