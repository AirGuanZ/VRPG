#pragma once

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

class Crosshair
{
    BlendState blend_;

public:

    Crosshair()
    {
        blend_ = BlendStateBuilder()
            .Set(0, true,
                D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_SUBTRACT,
                D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD)
            .Build();
    }

    void Draw(const Immediate2D &imm) const
    {
        Vec2i cen = imm.GetFramebufferSize() / 2;
        blend_.Bind();
        imm.DrawRectangleP(cen - Vec2i(2), cen + Vec2i(2), Vec4(1));
        blend_.Unbind();
    }
};

VRPG_WORLD_END
