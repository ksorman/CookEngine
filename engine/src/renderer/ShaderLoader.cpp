#include "ShaderLoader.h"
#include "spdlog/spdlog.h"
#include <string>

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
    }

    spdlog::info("[ShaderLoader] Initialize DXC compiler");
    hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
    if (FAILED(hres)) {
        spdlog::error("[ShaderLoader] Could not init DXC Compiler");
    }

    spdlog::info("[ShaderLoader] Initialize DXC utility");
    hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
    if (FAILED(hres)) {
        spdlog::error("[ShaderLoader] Could not init DXC Utiliy");
    }
}

Shader ShaderLoader::LoadShader(const std::wstring& fileName)
{
    HRESULT hres;
    spdlog::info(
      "[ShaderLoader] Load the HLSL text shader from disk: {}", std::string(fileName.begin(), fileName.end()));

    uint32_t codePage = DXC_CP_ACP;
    CComPtr<IDxcBlobEncoding> sourceBlob;
    hres = m_utils->LoadFile(fileName.c_str(), &codePage, &sourceBlob);
    if (FAILED(hres)) {
        spdlog::error("[ShaderLoader] Could not load shader file!");
    }

    ShaderType type;
    LPCWSTR targetProfile{};
    size_t idx = fileName.rfind('.', fileName.size() - 6);
    if (idx != std::string::npos) {
        std::wstring extension = fileName.substr(idx + 1);
        if (extension == L"vert.hlsl") {
            targetProfile = L"vs_6_1";
            type = ShaderType::VERTEX;
        }
        if (extension == L"frag.hlsl") {
            targetProfile = L"ps_6_1";
            type = ShaderType::FRAGMENT;
        }
    }

    std::vector<LPCWSTR> arguments = { fileName.c_str(), L"-E", L"main", L"-T", targetProfile, L"-spirv", L"-Zi" };

    DxcBuffer buffer{};
    buffer.Encoding = DXC_CP_ACP;
    buffer.Ptr = sourceBlob->GetBufferPointer();
    buffer.Size = sourceBlob->GetBufferSize();

    CComPtr<IDxcResult> result{ nullptr };
    hres = m_compiler->Compile(&buffer, arguments.data(), (uint32_t)arguments.size(), nullptr, IID_PPV_ARGS(&result));

    if (SUCCEEDED(hres)) {
        result->GetStatus(&hres);
    }

    if (FAILED(hres) && (result)) {
        CComPtr<IDxcBlobEncoding> errorBlob;
        hres = result->GetErrorBuffer(&errorBlob);
        if (SUCCEEDED(hres) && errorBlob) {
            spdlog::error("[ShaderLoader] Compilation failed!");
        }
    }

    CComPtr<IDxcBlob> code;
    result->GetResult(&code);

    return Shader(m_device, fileName, type, code->GetBufferPointer(), code->GetBufferSize());
}
}// namespace CookEngine