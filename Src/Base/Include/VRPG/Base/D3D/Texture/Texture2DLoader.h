#pragma once

#include <agz/utility/image.h>
#include <agz/utility/texture.h>

#include <VRPG/Base/D3D/D3DCreate.h>

VRPG_BASE_D3D_BEGIN

class Texture2DLoader
{
public:

    enum Format
    {
        RGB32,
        RGBA32
    };

    explicit Texture2DLoader(float invGamma = 1, Format format = RGBA32) noexcept;

    Texture2DLoader &SetInvGamma(float invGamma) noexcept;

    Texture2DLoader &SetFormat(Format format) noexcept;

    ComPtr<ID3D11ShaderResourceView> LoadFromFile(const std::string &filename) const;

private:

    ComPtr<ID3D11ShaderResourceView> LoadRGB32FromFile(const std::string &filename) const;

    ComPtr<ID3D11ShaderResourceView> LoadRGBA32FromFile(const std::string &filename) const;

    float invGamma_;

    Format format_;
};

inline Texture2DLoader::Texture2DLoader(float invGamma, Format format) noexcept
    : invGamma_(invGamma), format_(format)
{
    
}

inline Texture2DLoader &Texture2DLoader::SetInvGamma(float invGamma) noexcept
{
    invGamma_ = invGamma;
    return *this;
}

inline Texture2DLoader &Texture2DLoader::SetFormat(Format format) noexcept
{
    format_ = format;
    return *this;
}

inline ComPtr<ID3D11ShaderResourceView> Texture2DLoader::LoadFromFile(const std::string &filename) const
{
    switch(format_)
    {
    case RGB32:  return LoadRGB32FromFile(filename);
    case RGBA32: return LoadRGBA32FromFile(filename);
    }
    throw VRPGBaseException("unknown loading format");
}

inline ComPtr<ID3D11ShaderResourceView> Texture2DLoader::LoadRGB32FromFile(const std::string &filename) const
{
    auto dataf = agz::texture::texture2d_t<Vec3>(
        agz::img::load_rgb_from_file(filename).map(
        [&](const agz::math::color3b &c)
    {
        return Vec3(
                std::pow(c.r / 255.0f, invGamma_),
                std::pow(c.g / 255.0f, invGamma_),
                std::pow(c.b / 255.0f, invGamma_));
    }));

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width              = static_cast<UINT>(dataf.width());
    texDesc.Height             = static_cast<UINT>(dataf.height());
    texDesc.MipLevels          = 1;
    texDesc.ArraySize          = 1;
    texDesc.Format             = DXGI_FORMAT_R32G32B32_FLOAT;
    texDesc.SampleDesc.Count   = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage              = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags     = 0;
    texDesc.MiscFlags          = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem          = dataf.raw_data();
    initData.SysMemPitch      = dataf.width() * sizeof(Vec3);
    initData.SysMemSlicePitch = 0;

    ComPtr<ID3D11Texture2D> tex = CreateTexture2D(texDesc, &initData);
    if(!tex)
    {
        throw VRPGBaseException("failed to create texture2d for " + filename);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                    = DXGI_FORMAT_R32G32B32_FLOAT;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels       = 1;

    ComPtr<ID3D11ShaderResourceView> srv = CreateShaderResourceView(srvDesc, tex.Get());
    if(!srv)
    {
        throw VRPGBaseException("failed to create shader resource view for " + filename);
    }

    return srv;
}

inline ComPtr<ID3D11ShaderResourceView> Texture2DLoader::LoadRGBA32FromFile(const std::string &filename) const
{
    auto dataf = agz::texture::texture2d_t<Vec4>(
        agz::img::load_rgba_from_file(filename).map(
        [&](const agz::math::color4b &c)
    {
        return Vec4(
                std::pow(c.r / 255.0f, invGamma_),
                std::pow(c.g / 255.0f, invGamma_),
                std::pow(c.b / 255.0f, invGamma_),
                c.a / 255.0f);
    }));

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width              = static_cast<UINT>(dataf.width());
    texDesc.Height             = static_cast<UINT>(dataf.height());
    texDesc.MipLevels          = 1;
    texDesc.ArraySize          = 1;
    texDesc.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.SampleDesc.Count   = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage              = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags     = 0;
    texDesc.MiscFlags          = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem          = dataf.raw_data();
    initData.SysMemPitch      = dataf.width() * sizeof(Vec4);
    initData.SysMemSlicePitch = 0;

    ComPtr<ID3D11Texture2D> tex = CreateTexture2D(texDesc, &initData);
    if(!tex)
    {
        throw VRPGBaseException("failed to create texture2d for " + filename);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                    = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels       = 1;

    ComPtr<ID3D11ShaderResourceView> srv = CreateShaderResourceView(srvDesc, tex.Get());
    if(!srv)
    {
        throw VRPGBaseException("failed to create shader resource view for " + filename);
    }

    return srv;
}

VRPG_BASE_D3D_END
