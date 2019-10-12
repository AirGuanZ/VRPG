#pragma once

#include <map>
#include <string>

#include <agz/utility/misc.h>

#include <VRPG/Win/Shader/ShaderObjectBinding.h>

VRPG_WIN_BEGIN

template<ShaderStage STAGE>
class ShaderConstantBuffer
{
    UINT slot_;
    ComPtr<ID3D11Buffer> buffer_;

public:

    explicit ShaderConstantBuffer(UINT slot = 0) noexcept : slot_(slot) { }

    ShaderConstantBuffer(const ShaderConstantBuffer &copyFrom) = default;

    ShaderConstantBuffer(ShaderConstantBuffer &&moveFrom) noexcept = default;

    ShaderConstantBuffer &operator=(const ShaderConstantBuffer &copyFrom) = default;

    ShaderConstantBuffer &operator=(ShaderConstantBuffer &&moveFrom) noexcept = default;

    ~ShaderConstantBuffer() = default;

    void SetBuffer(ID3D11Buffer *buffer) noexcept
    {
        buffer_ = buffer;
    }

    void SetBuffer(ComPtr<ID3D11Buffer> buffer) noexcept
    {
        buffer_ = buffer;
    }

    ID3D11Buffer *GetBuffer() const noexcept
    {
        return buffer_.Get();
    }

    UINT GetSlot() const noexcept
    {
        return slot_;
    }

    void Bind() const
    {
        BindConstantBuffer<STAGE>(slot_, buffer_.Get());
    }

    void Unbind() const
    {
        BindConstantBuffer<STAGE>(slot_, nullptr);
    }
};

template<ShaderStage STAGE>
class ShaderConstantBufferManager : public agz::misc::uncopyable_t
{
public:

    struct ConstantBufferRecord
    {
        UINT byteSize = 0;
        ShaderConstantBuffer<STAGE> shaderConstantBuffer;

        ConstantBufferRecord()  = default;
        ~ConstantBufferRecord() = default;

        ConstantBufferRecord(const ConstantBufferRecord &)            = default;
        ConstantBufferRecord &operator=(const ConstantBufferRecord &) = default;

        ConstantBufferRecord(ConstantBufferRecord &&)            noexcept = default;
        ConstantBufferRecord &operator=(ConstantBufferRecord &&) noexcept = default;
    };

    using ConstantBufferRecordTable = std::map<std::string, ConstantBufferRecord>;

    ShaderConstantBufferManager() = default;

    explicit ShaderConstantBufferManager(ConstantBufferRecordTable &&table) noexcept
        : table_(std::move(table))
    {
        
    }

    bool Add(std::string name, UINT slot, UINT byteSize)
    {
        assert(byteSize > 0);
        if(table_.find(name) != table_.end())
            return false;
        table_.insert(std::make_pair(
            std::move(name), ConstantBufferRecord{ byteSize, ShaderConstantBuffer<STAGE>(slot) }));
        return true;
    }

    ShaderConstantBuffer<STAGE> *Get(const std::string &name)
    {
        auto it = table_.find(name);
        return it != table_.end() ? &it->second.shaderConstantBuffer : nullptr;
    }

    void Bind()
    {
        for(auto &it : table_)
            it.second.shaderConstantBuffer.Bind();
    }

    void Unbind()
    {
        for(auto &it : table_)
            it.second.shaderConstantBuffer.Unbind();
    }

private:

    ConstantBufferRecordTable table_;
};

VRPG_WIN_END
