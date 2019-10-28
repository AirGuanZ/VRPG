#pragma once

#include <cassert>

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

class ScalarHistory
{
    std::vector<float> history_;
    int nextPosition_;

public:

    explicit ScalarHistory(int size = 4)
        : history_(size, 0), nextPosition_(0)
    {
        assert(size > 0);
    }

    void Update(float newValue) noexcept
    {
        history_[nextPosition_] = newValue;
        nextPosition_ = (nextPosition_ + 1) % int(history_.size());
    }

    float MeanValue() const noexcept
    {
        float sum = 0;
        for(auto v : history_)
            sum += v;
        return sum / history_.size();
    }

    void Clear()
    {
        for(auto &v : history_)
            v = 0;
    }
};

VRPG_WORLD_END
