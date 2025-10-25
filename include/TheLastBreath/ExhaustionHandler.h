#pragma once
#include <mutex>

namespace TheLastBreath {

    class ExhaustionHandler {
    public:
        static ExhaustionHandler* GetSingleton() {
            static ExhaustionHandler singleton;
            return &singleton;
        }

        void Update();
        void ClearAll();

    private:
        ExhaustionHandler() = default;
        ExhaustionHandler(const ExhaustionHandler&) = delete;
        ExhaustionHandler(ExhaustionHandler&&) = delete;

        struct ExhaustionState {
            bool isExhausted = false;
            // Store DELTAS to revert on removal
            float originalSpeed = 0.0f;          // Speed delta applied
            float originalAttackDamage = 0.0f;   // Attack damage delta applied
        };

        std::unordered_map<RE::FormID, ExhaustionState> actorStates;
        mutable std::mutex statesMutex; 

        void ApplyExhaustion(RE::Actor* actor);
        void RemoveExhaustion(RE::Actor* actor);
    };

}
