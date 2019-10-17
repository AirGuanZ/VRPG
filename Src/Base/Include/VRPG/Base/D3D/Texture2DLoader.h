#pragma once

#include <agz/utility/image.h>

#include <VRPG/Base/D3D/D3DCreate.h>

VRPG_BASE_D3D_BEGIN

class Texture2DLoader
{
public:

    static ComPtr<ID3D11ShaderResourceView> LoadShaderResourceViewFromImage(const std::string &filename);
};

VRPG_BASE_D3D_END
