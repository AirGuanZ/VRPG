#pragma once

#include <ostream>
#include <type_traits>

#include <VRPG/Mesh/Common.h>

VRPG_MESH_BEGIN

inline void WriteOStream(std::ostream &out, const void *data, size_t byteCount)
{
    out.write(static_cast<const char *>(data), byteCount);
    if(!out)
    {
        throw VRPGMeshException("failed to write to ostream");
    }
}

template<typename T>
void WriteOStream(std::ostream &out, const T &value)
{
    static_assert(std::is_trivially_copyable_v<T>);
    WriteOStream(out, &value, sizeof(T));
}

template<>
inline void WriteOStream<std::string>(std::ostream &out, const std::string &str)
{
    uint32_t strLen = static_cast<uint32_t>(str.size());
    WriteOStream(out, strLen);
    WriteOStream(out, str.data(), str.size());
}

inline void ReadIStream(std::istream &in, void *data, size_t byteSize)
{
    in.read(static_cast<char *>(data), byteSize);
    if(!in)
    {
        throw VRPGMeshException("failed to read " + std::to_string(byteSize) + " bytes from istream");
    }
}

template<typename T>
void ReadIStream(std::istream &in, T &value)
{
    static_assert(std::is_trivially_copyable_v<T>);
    ReadIStream(in, &value, sizeof(T));
}

template<>
inline void ReadIStream<std::string>(std::istream &in, std::string &str)
{
    uint32_t strLen;
    ReadIStream(in, strLen);
    str.resize(strLen + 1);
    ReadIStream(in, str.data(), strLen);
}

VRPG_MESH_END
