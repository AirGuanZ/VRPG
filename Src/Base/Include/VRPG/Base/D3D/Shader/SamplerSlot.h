#pragma once

#include <map>
#include <string>

#include <VRPG/Base/D3D/Shader/ObjectBinding.h>

VRPG_BASE_D3D_BEGIN

template<ShaderStage STAGE>
class SamplerSlot
{
    UINT slot_;
    ComPtr<ID3D11SamplerState> sampler_;

public:

    explicit SamplerSlot(UINT slot = 0) noexcept : slot_(slot) { }

    SamplerSlot(const SamplerSlot &)            = default;
    SamplerSlot &operator=(const SamplerSlot &) = default;

    SamplerSlot(SamplerSlot &&)            noexcept = default;
    SamplerSlot &operator=(SamplerSlot &&) noexcept = default;

    ~SamplerSlot() = default;

    void SetSampler(ID3D11SamplerState *sampler)
    {
        sampler_ = sampler;
    }

    void SetSampler(ComPtr<ID3D11SamplerState> sampler)
    {
        sampler_ = sampler;
    }

    UINT GetSlot() const noexcept
    {
        return slot_;
    }

    void Bind() const
    {
        ObjectBinding::BindSampler<STAGE>(slot_, sampler_.Get());
    }

    void Unbind() const
    {
        ObjectBinding::BindSampler<STAGE>(slot_, nullptr);
    }
};

template<ShaderStage STAGE>
class SamplerSlotManager
{
public:

    struct Record
    {
        SamplerSlot<STAGE> samplerSlot;

        Record() = default;
        ~Record() = default;

        Record(const Record &) = default;
        Record &operator=(const Record &) = default;

        Record(Record &&)            noexcept = default;
        Record &operator=(Record &&) noexcept = default;
    };

    using RecordTable = std::map<std::string, Record>;

    SamplerSlotManager() = default;
    ~SamplerSlotManager() = default;

    SamplerSlotManager(const SamplerSlotManager &)            = default;
    SamplerSlotManager &operator=(const SamplerSlotManager &) = default;

    SamplerSlotManager(SamplerSlotManager &&)            noexcept = default;
    SamplerSlotManager &operator=(SamplerSlotManager &&) noexcept = default;

    explicit SamplerSlotManager(RecordTable table) noexcept
        : table_(std::move(table))
    {

    }

    bool Add(std::string name, UINT slot)
    {
        if(table_.find(name) != table_.end())
        {
            return false;
        }
        table_.insert(std::make_pair(
            std::move(name), Record{ SamplerSlot<STAGE>(slot) }));
        return true;
    }

    SamplerSlot<STAGE> *Get(const std::string &name)
    {
        auto it = table_.find(name);
        return it != table_.end() ? &it->second.samplerSlot : nullptr;
    }

    const SamplerSlot<STAGE> *Get(const std::string &name) const
    {
        auto it = table_.find(name);
        return it != table_.end() ? &it->second.samplerSlot : nullptr;
    }

    void Bind() const
    {
        for(auto &it : table_)
        {
            it.second.samplerSlot.Bind();
        }
    }

    void Unbind() const
    {
        for(auto &it : table_)
        {
            it.second.samplerSlot.Unbind();
        }
    }

private:

    RecordTable table_;
};

VRPG_BASE_D3D_END
