#pragma once
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace TheLastBreath {

    // Consolidated state for all combat systems
    struct ActorCombatState {
        // Slow Motion System
        bool bowSlowActive = false;
        bool castLeftActive = false;
        bool castRightActive = false;
        bool dualCastActive = false;
        float originalSpeedMult = 1.0f;
        std::chrono::steady_clock::time_point lastCastTime;

        // Ranged Stamina System
        bool isRangedDrawn = false;
        std::chrono::steady_clock::time_point drawStartTime;
        std::chrono::steady_clock::time_point lastRangedDrainTime;

        // Exhaustion System
        bool isExhausted = false;
        float originalExhaustionSpeed = 0.0f;
        float originalExhaustionAttackDmg = 0.0f;
        float originalExhaustionDamageResist = 0.0f;

        // Timed Block System
        bool isButtonPressed = false;
        bool windowActive = false;
        bool windowConsumed = false;
        std::chrono::steady_clock::time_point buttonPressTime;
        std::chrono::steady_clock::time_point windowStartTime;

        // Block Stamina Drain
        bool isBlocking = false;
        std::chrono::steady_clock::time_point blockStartTime;
        std::chrono::steady_clock::time_point lastBlockDrainTime;

        // Damage Resistance (Timed Block)
        float pendingResistAmount = 0.0f;
        std::chrono::steady_clock::time_point resistApplyTime;

        // Block Effects (Sound/Spark counter)
        uint32_t consecutiveParryCount = 0;
    };

    class ActorStateManager {
    public:
        static ActorStateManager* GetSingleton() {
            static ActorStateManager singleton;
            return &singleton;
        }

        // Thread-safe state access
        ActorCombatState* GetState(RE::Actor* actor);
        ActorCombatState* GetStateIfExists(RE::Actor* actor);

        void ClearActor(RE::Actor* actor);
        void ClearAll();

    private:
        ActorStateManager() = default;
        ActorStateManager(const ActorStateManager&) = delete;
        ActorStateManager(ActorStateManager&&) = delete;

        std::unordered_map<RE::FormID, ActorCombatState> actorStates;
        std::mutex stateMutex;
    };
}
