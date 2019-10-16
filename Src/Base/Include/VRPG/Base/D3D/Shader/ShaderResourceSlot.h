#pragma once

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include <VRPG/Base/D3D/Shader/ObjectBinding.h>

VRPG_BASE_D3D_BEGIN

template<ShaderStage STAGE>
class ShaderResourceSlot
{
    UINT slot_;
    mutable std::vector<ID3D11ShaderResourceView*> srvs_;

    void AddRef() noexcept
    {
        for(auto srv : srvs_)
        {
            if(srv)
                srv->AddRef();
        }
    }

    void DecRef() noexcept
    {
        for(auto srv : srvs_)
        {
            if(srv)
                srv->Release();
        }
    }

public:

    explicit ShaderResourceSlot(UINT slot = 0, UINT srvCount = 1)
        : slot_(slot), srvs_(srvCount, nullptr)
    {
        
    }

    ShaderResourceSlot(const ShaderResourceSlot &copyFrom)
    {
        slot_ = copyFrom.slot_;
        srvs_ = copyFrom.srvs_;
        AddRef();
    }

    ShaderResourceSlot &operator=(const ShaderResourceSlot &copyFrom)
    {
        DecRef();
        slot_ = copyFrom.slot_;
        srvs_ = copyFrom.srvs_;
        AddRef();
        return *this;
    }

    ShaderResourceSlot(ShaderResourceSlot &&moveFrom) noexcept
    {
        slot_ = moveFrom.slot_;
        srvs_ = std::move(moveFrom.srvs_);
        moveFrom.slot_ = 0;
    }

    ShaderResourceSlot &operator=(ShaderResourceSlot &&moveFrom) noexcept
    {
        DecRef();
        slot_ = moveFrom.slot_;
        srvs_ = std::move(moveFrom.srvs_);
        moveFrom.slot_ = 0;
        return *this;
    }

    ~ShaderResourceSlot()
    {
        DecRef();
    }

    int GetSRVCount() const noexcept
    {
        return int(srvs_.size());
    }

    void SetShaderResourceView(ID3D11ShaderResourceView *srv)
    {
        assert(GetSRVCount() == 1);
        SetShaderResourceView(0, srv);
    }

    void SetShaderResourceView(int index, ID3D11ShaderResourceView *srv)
    {
        assert(0 <= index && index < GetSRVCount());
        auto &dst = srvs_[index];
        if(dst)
            dst->Release();
        dst = srv;
        if(srv)
            srv->AddRef();
    }

    void SetShaderResourceView(int start, int count, ID3D11ShaderResourceView **srvs)
    {
        assert(0 <= start && 0 < count && start + count < GetSRVCount());
        for(int i = 0, j = start; i < count; ++i, ++j)
            SetShaderResourceView(j, srvs[i]);
    }

    void Bind() const
    {
        ObjectBinding::BindShaderResourceViewArray<STAGE>(slot_, UINT(srvs_.size()), srvs_.data());
    }

    void Unbind() const
    {
        if(srvs_.size() < 20)
        {
            static ID3D11ShaderResourceView *EMPTY_SRVS[20] = { nullptr };
            ObjectBinding::BindShaderResourceViewArray<STAGE>(slot_, UINT(srvs_.size()), EMPTY_SRVS);
        }
        else
        {
            std::vector<ID3D11ShaderResourceView *> emptySRVs(srvs_.size());
            ObjectBinding::BindShaderResourceViewArray<STAGE>(slot_, UINT(srvs_.size()), emptySRVs.data());
        }
    }
};

template<ShaderStage STAGE>
class ShaderResourceSlotManager
{
public:

    struct Record
    {
        ShaderResourceSlot<STAGE> shaderResourceViewSlot;
        
        Record()  = default;
        ~Record() = default;

        Record(const Record &)            = default;
        Record &operator=(const Record &) = default;

        Record(Record &&)            noexcept = default;
        Record &operator=(Record &&) noexcept = default;
    };

    using RecordTable = std::map<std::string, Record>;

    ShaderResourceSlotManager() = default;
    ~ShaderResourceSlotManager() = default;

    ShaderResourceSlotManager(const ShaderResourceSlotManager &)            = default;
    ShaderResourceSlotManager &operator=(const ShaderResourceSlotManager &) = default;

    ShaderResourceSlotManager(ShaderResourceSlotManager &&)            noexcept = default;
    ShaderResourceSlotManager &operator=(ShaderResourceSlotManager &&) noexcept = default;

    explicit ShaderResourceSlotManager(RecordTable table) noexcept
        : table_(std::move(table))
    {
        
    }

    bool Add(std::string name, UINT slot, UINT count)
    {
        if(table_.find(name) != table_.end())
            return false;
        table_.insert(std::make_pair(
            std::move(name), Record{ ShaderResourceSlot<STAGE>(slot, count) }));
        return true;
    }

    ShaderResourceSlot<STAGE> *Get(const std::string &name)
    {
        auto it = table_.find(name);
        return it != table_.end() ? &it->second.shaderResourceViewSlot : nullptr;
    }

    const ShaderResourceSlot<STAGE> *Get(const std::string &name) const
    {
        auto it = table_.find(name);
        return it != table_.end() ? &it->second.shaderResourceViewSlot : nullptr;
    }

    void Bind() const
    {
        for(auto &it : table_)
            it.second.shaderResourceViewSlot.Bind();
    }

    void Unbind() const
    {
        for(auto &it : table_)
            it.second.shaderResourceViewSlot.Unbind();
    }

private:

    RecordTable table_;
};

VRPG_BASE_D3D_END
