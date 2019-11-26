#pragma once

#include <type_traits>

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

class AnimatedMeshIndexBuffer
{
public:

    virtual ~AnimatedMeshIndexBuffer() = default;

    virtual void Bind() const;

    virtual void Unbind() const;

    virtual UINT GetIndexCount() const noexcept = 0;
};

template<typename T>
class AnimatedMeshIndexBufferImpl : public AnimatedMeshIndexBuffer
{
    static_assert(std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>);

    IndexBuffer<T> indexBuffer_;

public:

    AnimatedMeshIndexBufferImpl(const T *data, UINT indexCount)
        : indexBuffer_(indexCount, false, data)
    {

    }

    void Bind() const override
    {
        indexBuffer_.Bind();
    }

    void Unbind() const override
    {
        indexBuffer_.Unbind();
    }

    UINT GetIndexCount() const noexcept override
    {
        return indexBuffer_.GetIndexCount();
    }
};

VRPG_GAME_END
