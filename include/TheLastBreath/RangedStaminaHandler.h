#pragma once
#include <unordered_map>
#include <chrono>

namespace TheLastBreath {
    class RangedStaminaHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        static RangedStaminaHandler* GetSingleton() {
            static RangedStaminaHandler singleton;
            return &singleton;
        }

        void OnRangedDrawn(RE::Actor* actor);
        void OnRangedRelease(RE::Actor* actor);
        void OnBlockStart(RE::Actor* actor);
        void OnBlockStop(RE::Actor* actor);
        void Update();

        bool IsActorTracked(RE::Actor* actor) const;
        void ClearActor(RE::Actor* actor);

        virtual RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent* a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;

    private:
        RangedStaminaHandler() = default;
        RangedStaminaHandler(const RangedStaminaHandler&) = delete;
        RangedStaminaHandler(RangedStaminaHandler&&) = delete;

        struct DrawState {
            bool isDrawn = false;
			bool isBlocking = false; // blocking stamina drain added here
            std::chrono::steady_clock::time_point drawStartTime;
            std::chrono::steady_clock::time_point lastDrainTime;
			std::chrono::steady_clock::time_point blockStartTime; // blocking stamina drain added here
			std::chrono::steady_clock::time_point lastBlockDrainTime; // blocking stamina drain added here
            RE::ActorHandle handle;
        };

        std::unordered_map<RE::FormID, DrawState> actorStates;
        std::chrono::steady_clock::time_point lastUpdateTime;
    };
}
