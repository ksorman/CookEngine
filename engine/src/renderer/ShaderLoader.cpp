#include "ShaderLoader.h"
#include "spdlog/spdlog.h"
#include <string>
#include <vector>

namespace CookEngine {

ShaderLoader::ShaderLoader(VkDevice device)
{
    Init(device);
}

void ShaderLoader::Init(VkDevice device)
{
    m_device = device;

    HRESULT hres;

    spdlog::info("[ShaderLoader] Initialize DXC library");
    hres = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library));
    if (FAILED(hres)) {
        spdlog::error("[ShaderLoader] Could not init DXC Library");
        return;
    }

    spdlog::info("[ShaderLoader] Initialize DXC compiler");
    hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
    if (FAILED(hres)) {
        spdlog::error("[ShaderLoader] Could not init DXC Compiler");
        return;
    }

    spdlog::info("[ShaderLoader] Initialize DXC utility");
    hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
    if (FAILED(hres)) {
        spdlog::error("[ShaderLoader] Could not init DXC Utility");
        return;
    }
}

std::unique_ptr<Shader> ShaderLoader::LoadShader(const std::wstring& fileName)
{
    if (!m_compiler || !m_utils) {
        spdlog::error("[ShaderLoader] Must be initialized before loading shaders");
        return nullptr;
    }

    ShaderType type = ShaderType::UNKNOWN;
    LPCWSTR targetProfile = nullptr;
    if (fileName.ends_with(L".vert.hlsl")) {
        targetProfile = L"vs_6_1";
        type = ShaderType::VERTEX;
    } else if (fileName.ends_with(L".frag.hlsl")) {
        targetProfile = L"ps_6_1";
        type = ShaderType::FRAGMENT;
    } else {
        spdlog::error("[ShaderLoader] Unsupported shader extension: expected .vert.hlsl or .frag.hlsl");
        return nullptr;
    }

    HRESULT hres;
    const std::string shaderName(fileName.begin(), fileName.end());
    spdlog::info("[ShaderLoader] Load the HLSL text shader from disk: {}", shaderName);

    uint32_t codePage = DXC_CP_ACP;
    CComPtr<IDxcBlobEncoding> sourceBlob;
    hres = m_utils->LoadFile(fileName.c_str(), &codePage, &sourceBlob);
    if (FAILED(hres) || !sourceBlob) {
        const auto message = fmt::format(
          "[ShaderLoader] Could not load '{}': HRESULT {:#x}", shaderName, static_cast<uint32_t>(hres));
        spdlog::error("{}", message);
        return nullptr;
    }

    std::vector<LPCWSTR> arguments = { fileName.c_str(), L"-E", L"main", L"-T", targetProfile, L"-spirv", L"-Zi" };

    DxcBuffer buffer{};
    buffer.Encoding = codePage;
    buffer.Ptr = sourceBlob->GetBufferPointer();
    buffer.Size = sourceBlob->GetBufferSize();

    CComPtr<IDxcResult> result{ nullptr };
    hres = m_compiler->Compile(&buffer, arguments.data(), (uint32_t)arguments.size(), nullptr, IID_PPV_ARGS(&result));
    if (FAILED(hres) || !result) {
        const auto message = fmt::format(
          "[ShaderLoader] DXC invocation failed for '{}': HRESULT {:#x}", shaderName, static_cast<uint32_t>(hres));
        spdlog::error("{}", message);
        return nullptr;
    }

    HRESULT status = E_FAIL;
    hres = result->GetStatus(&status);
    if (FAILED(hres) || FAILED(status)) {
        CComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        const auto message = fmt::format("[ShaderLoader] Compilation failed for '{}': {}", shaderName,
          errors && errors->GetStringLength() ? errors->GetStringPointer() : "DXC did not provide diagnostics");
        spdlog::error("{}", message);
        return nullptr;
    }

    CComPtr<IDxcBlob> code;
    hres = result->GetResult(&code);
    if (FAILED(hres) || !code || code->GetBufferSize() == 0) {
        spdlog::error("[ShaderLoader] No compiled SPIR-V for '{}'", shaderName);
        return nullptr;
    }

    return std::make_unique<Shader>(m_device, fileName, type, code->GetBufferPointer(), code->GetBufferSize());
}
}// namespace CookEngine
