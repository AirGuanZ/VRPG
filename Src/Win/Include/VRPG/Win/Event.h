#pragma once

#include <cassert>
#include <functional>
#include <set>
#include <unordered_set>

#include <agz/utility/misc.h>

#include <VRPG/Win/Common.h>

VRPG_WIN_BEGIN

template<typename Event>
class EventHandlerSet;

template<typename Event>
class EventHandler : public agz::misc::uncopyable_t
{
    friend class EventHandlerSet<Event>;

    std::set<EventHandlerSet<Event>*> containedSets_;

public:

    virtual ~EventHandler();

    virtual void Handle(const Event &param) = 0;
};

template<typename Event>
class EventHandlerSet : public agz::misc::uncopyable_t
{
    std::unordered_set<EventHandler<Event>*> handlers_;

public:

    ~EventHandlerSet()
    {
        DetachAll();
    }

    void InvokeAllHandlers(const Event &param) const
    {
        for(auto handler : handlers_)
            handler->Handle(param);
    }

    void Attach(EventHandler<Event> *handler)
    {
        assert(handler);
        handler->containedSets_.insert(this);
        handlers_.insert(handler);
    }

    void Detach(EventHandler<Event> *handler)
    {
        assert(handler);
        handler->containedSets_.erase(this);
        handlers_.erase(handler);
    }

    void DetachAll()
    {
        while(!handlers_.empty())
            Detach(*handlers_.begin());
    }
};

template<typename Event>
EventHandler<Event>::~EventHandler()
{
    while(!containedSets_.empty())
        (*containedSets_.begin())->Detach(this);
}

template<typename...Events>
class EventManager : public agz::misc::uncopyable_t
{
    std::tuple<EventHandlerSet<Events>...> handlerSets_;

public:

    template<typename Event>
    void InvokeAllHandlers(const Event &param)
    {
        std::get<EventHandlerSet<Event>>(handlerSets_).InvokeAllHandlers(param);
    }

    template<typename Event>
    void Attach(EventHandler<Event> *handler)
    {
        std::get<EventHandlerSet<Event>>(handlerSets_).Attach(handler);
    }

    template<typename Event>
    void Detach(EventHandler<Event> *handler)
    {
        std::get<EventHandlerSet<Event>>(handlerSets_).Detach(handler);
    }

    template<typename Event>
    void DetachAll()
    {
        std::get<EventHandlerSet<Event>>(handlerSets_).DetachAll();
    }

    void DetachAllTypes()
    {
        std::apply(
            [](auto&&...handlerSet){ ((handlerSet.DetachAll()), ...); },
            handlerSets_);
    }
};

// ========== predefined event handlers ==========

template<typename Event>
class FunctionalEventHandler : public EventHandler<Event>
{
public:

    using Function = std::function<void(const Event &)>;

    explicit FunctionalEventHandler(Function f = Function())
        : func_(std::move(f))
    {
        
    }

    void SetFunction(Function f)
    {
        func_ = std::move(f);
    }

    void Handle(const Event &param) override
    {
        if(func_)
            func_(param);
    }

private:

    Function func_;
};

VRPG_WIN_END
