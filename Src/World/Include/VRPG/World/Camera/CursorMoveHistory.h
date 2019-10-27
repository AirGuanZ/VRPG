#pragma once

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

class CursorMoveHistory
{
public:

    static constexpr int HISTORY_SIZE = 12;

    void Update(float value) noexcept
    {
        history_[nextPosition_] = value;
        nextPosition_ = (nextPosition_ + 1) % HISTORY_SIZE;
    }

    float HistoryMean() const noexcept
    {
        float sum = 0;
        for(float v : history_)
            sum += v;
        return 1 / float(HISTORY_SIZE) * sum;
    }

    void Clear() noexcept
    {
        for(float &v : history_)
            v = 0;
        nextPosition_ = 0;
    }

private:

    float history_[HISTORY_SIZE] = { 0 };
    int nextPosition_ = 0;
};

VRPG_WORLD_END
