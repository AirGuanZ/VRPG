#pragma once

#include <agz/utility/misc.h>

#include <VRPG/Base/Common.h>

VRPG_BASE_BEGIN

template<typename T>
class Singleton : public agz::misc::uncopyable_t
{
public:

    static T &GetInstance()
    {
        static T ret;
        return ret;
    }
};

VRPG_BASE_END
