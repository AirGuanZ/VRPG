#pragma once

#include <VRPG/Base/Common.h>

VRPG_BASE_D3D_BEGIN

template<typename T>
class ComObjectHolder
{
protected:

    ComPtr<T> obj_;

public:

    ComObjectHolder() = default;

    explicit ComObjectHolder(T *obj) noexcept
        : obj_(obj)
    {

    }

    explicit ComObjectHolder(ComPtr<T> obj) noexcept
        : obj_(std::move(obj))
    {

    }

    ComObjectHolder &operator=(T *obj) noexcept
    {
        obj_ = obj;
        return *this;
    }


    ComObjectHolder &operator=(ComPtr<T> obj) noexcept
    {
        obj_ = std::move(obj);
        return *this;
    }

    T *Get() const noexcept
    {
        return obj_.Get();
    }

    operator T *() const noexcept
    {
        return obj_.Get();
    }
};

VRPG_BASE_D3D_END
