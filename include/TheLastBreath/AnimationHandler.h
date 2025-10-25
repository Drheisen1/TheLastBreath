#pragma once

namespace TheLastBreath {
    class AnimationEventHandler : public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    public:
        static AnimationEventHandler* GetSingleton();


        RE::BSEventNotifyControl ProcessEvent(
            const RE::BSAnimationGraphEvent* a_event,
            RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override;

    private:
        AnimationEventHandler() = default;
        AnimationEventHandler(const AnimationEventHandler&) = delete;
        AnimationEventHandler(AnimationEventHandler&&) = delete;

        void OnBowDrawn(RE::Actor* actor);
    };
}
