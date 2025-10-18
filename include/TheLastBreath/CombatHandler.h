#pragma once
#include <chrono>

namespace TheLastBreath {

    class CombatHandler {
    public:
        static CombatHandler* GetSingleton() {
            static CombatHandler singleton;
            return &singleton;
        }

        void OnActorHit(RE::Actor* victim, RE::Actor* aggressor, float damage);
        void Update();  // ADD THIS - called from worker thread

    private:
        CombatHandler() = default;
        CombatHandler(const CombatHandler&) = delete;
        CombatHandler(CombatHandler&&) = delete;

        float GetArmorSkill(RE::Actor* actor);

        // gradual
        struct StaminaDrain {
            float totalAmount;
            float remaining;
            std::chrono::steady_clock::time_point startTime;
            std::chrono::steady_clock::time_point lastDrainTime;
            float duration;  // in seconds
        };

        std::unordered_map<RE::FormID, StaminaDrain> activeDrains;  // ADD THIS
    };

}