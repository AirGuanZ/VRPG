#include <codecvt>
#include <locale>

#include <wincodec.h>

#include <VRPG/Base/D3D/Texture2DLoader.h>

#include "./WICTextureLoader.h"

VRPG_BASE_D3D_BEGIN

namespace
{
    struct WICInitializer
    {
        WICInitializer()
        {
            HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
            if(FAILED(hr))
                throw VRPGBaseException("failed to initialize WIC texture loader");
        }
    };
}

ComPtr<ID3D11ShaderResourceView> Texture2DLoader::LoadShaderResourceViewFromImage(const std::string &filename)
{
    [[maybe_unused]] static WICInitializer initializer;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wideFilename = converter.from_bytes(filename);

    ComPtr<ID3D11ShaderResourceView> srv;
    HRESULT hr = DirectX::CreateWICTextureFromFile(gDevice, wideFilename.c_str(), nullptr, srv.GetAddressOf());
    if(FAILED(hr))
        throw VRPGBaseException("failed to create shader resource view with image file: " + filename);
    return srv;
}

VRPG_BASE_D3D_END
