#pragma once

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
            // Store ORIGINAL values before exhaustion to avoid corruption
            float originalSpeed = 0.0f;
            float originalAttackDamage = 0.0f;
            float originalDamageResist = 0.0f;
        };

        std::unordered_map<RE::FormID, ExhaustionState> actorStates;

        void ApplyExhaustion(RE::Actor* actor);
        void RemoveExhaustion(RE::Actor* actor);
    };

}
