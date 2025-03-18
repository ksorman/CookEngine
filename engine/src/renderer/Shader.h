#ifndef SHADER_H
#define SHADER_H

#include <cstddef>
#include <string>

#include "VmaUsage.h"

namespace CookEngine {

enum class ShaderType{
    UNKNOWN = 0,
    VERTEX = 1,
    FRAGMENT = 2,
};

class Shader
{
  public:
    Shader(VkDevice device, std::wstring_view fileName, ShaderType type, void* shaderBlobPtr, size_t shaderBlobSize);
    ~Shader();

    VkShaderModule GetShaderModul() const;
    operator VkShaderModule() const;
    VkPipelineShaderStageCreateInfo GetPiplineShaderStageCreateInfo() const;
    operator VkPipelineShaderStageCreateInfo() const;

  private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkShaderModule m_shaderModule = VK_NULL_HANDLE;
    ShaderType m_type = ShaderType::UNKNOWN;
    const std::wstring m_fileName;
};
}// namespace CookEngine

#endif// SHADER_H
