#pragma once
#include "TheLastBreath/TimedBlockHandler.h"
#include "TheLastBreath/BlockEffectsHandler.h"
#include <unordered_set>
#include <unordered_map>
#include <chrono>

namespace TheLastBreath {

    class CombatHandler {
    public:
        static CombatHandler* GetSingleton() {
            static CombatHandler singleton;
            return &singleton;
        }

        void OnActorHit(RE::Actor* victim, RE::Actor* aggressor, float damage, BlockType blockType);
        void ApplyTimedBlockDamageResistance(RE::Actor* victim);

        // Block stamina drain
        void OnBlockStart(RE::Actor* actor);
        void OnBlockStop(RE::Actor* actor);
        void Update();

    private:
        CombatHandler() = default;
        CombatHandler(const CombatHandler&) = delete;
        CombatHandler(CombatHandler&&) = delete;

        struct DamageResistanceState {
            float resistAmount = 0.0f;
            std::chrono::steady_clock::time_point applyTime;
        };

        struct BlockState {
            bool isBlocking = false;
            std::chrono::steady_clock::time_point blockStartTime;
            std::chrono::steady_clock::time_point lastBlockDrainTime;
        };

        std::unordered_map<RE::FormID, DamageResistanceState> actorsWithDamageResistance;
        std::unordered_map<RE::FormID, BlockState> actorBlockStates;
    };

}