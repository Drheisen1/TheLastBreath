#pragma once

namespace TheLastBreath {

    class HitEventHandler : public RE::BSTEventSink<RE::TESHitEvent> {
    public:
        static HitEventHandler* GetSingleton() {
            static HitEventHandler singleton;
            return &singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::TESHitEvent* a_event,
            RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override;

    private:
        HitEventHandler() = default;
        HitEventHandler(const HitEventHandler&) = delete;
        HitEventHandler(HitEventHandler&&) = delete;
    };

}