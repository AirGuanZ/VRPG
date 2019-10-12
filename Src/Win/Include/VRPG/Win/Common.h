#pragma once

#include <stdexcept>

#include <wrl/client.h>
#include <d3d11.h>

#define VRPG_WIN_BEGIN namespace vrpg::win {
#define VRPG_WIN_END   }

VRPG_WIN_BEGIN

class VRPGWinException : public std::runtime_error
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

VRPG_WIN_END
