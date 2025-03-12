#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include "Shader.h"
#include <atlbase.h>
#include <dxcapi.h>

namespace CookEngine {
class ShaderLoader
{
  public:
    ShaderLoader() = default;
    ShaderLoader(VkDevice device);
    void Init(VkDevice device);
    Shader LoadShader(const std::wstring& fileName);

  private:
    VkDevice m_device = VK_NULL_HANDLE;
    CComPtr<IDxcLibrary> m_library;
    CComPtr<IDxcCompiler3> m_compiler;
    CComPtr<IDxcUtils> m_utils;
};
}// namespace CookEngine
#endif// SHADER_LOADER_H