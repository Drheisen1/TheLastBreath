#pragma once
#include <chrono>
#include <unordered_map>

namespace TheLastBreath {

    class RangedStaminaHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        static RangedStaminaHandler* GetSingleton() {
            static RangedStaminaHandler singleton;
            return &singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent* a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;

        void OnRangedDrawn(RE::Actor* actor);
        void OnRangedRelease(RE::Actor* actor);
        void Update();

        bool IsActorTracked(RE::Actor* actor) const;
        void ClearActor(RE::Actor* actor);

    private:
        RangedStaminaHandler() = default;
        RangedStaminaHandler(const RangedStaminaHandler&) = delete;
        RangedStaminaHandler(RangedStaminaHandler&&) = delete;

        struct ActorState {
            bool isDrawn = false;
            std::chrono::steady_clock::time_point drawStartTime;
            std::chrono::steady_clock::time_point lastDrainTime;
            RE::ObjectRefHandle handle;
        };

        std::unordered_map<RE::FormID, ActorState> actorStates;
    };

}