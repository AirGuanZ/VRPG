#pragma once

#include <stdexcept>

#include <wrl/client.h>
#include <d3d11.h>

#define VRPG_BASE_BEGIN namespace VRPG::Base {
#define VRPG_BASE_END   }

#define VRPG_BASE_D3D_BEGIN namespace VRPG::Base::D3D {
#define VRPG_BASE_D3D_END   }

VRPG_BASE_BEGIN

class VRPGBaseException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

using Microsoft::WRL::ComPtr;

template<typename T, typename = std::enable_if_t<std::is_pointer_v<std::remove_reference_t<T>>>>
void ReleaseCOMObjects(T &&ptr)
{
    if(ptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

template<typename T0, typename T1, typename...Ts>
void ReleaseCOMObjects(T0 &&ptr0, T1 &&ptr1, Ts&&...ptrs)
{
    ReleaseCOMObjects(std::forward<T0>(ptr0));
    ReleaseCOMObjects(std::forward<T1>(ptr1), std::forward<Ts>(ptrs)...);
}

inline ID3D11Device        *gDevice        = nullptr;
inline ID3D11DeviceContext *gDeviceContext = nullptr;

VRPG_BASE_END
