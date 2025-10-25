#pragma once
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace TheLastBreath {

    enum class BlockEquipmentType {
        Shield,
        Weapon,
        None
    };

    class BlockEffectsHandler {
    public:
        static BlockEffectsHandler* GetSingleton() {
            static BlockEffectsHandler singleton;
            return &singleton;
        }

        // Call this on successful timed block
        void OnSuccessfulTimedBlock(RE::Actor* blocker, RE::Actor* aggressor);

        // Call this when timed block fails or regular block happens
        void OnTimedBlockFailed(RE::Actor* blocker);

        // Clear tracking for an actor
        void ClearActor(RE::Actor* actor);

        // Update function for timeout checking
        void Update();

        // Get current parry level for an actor (0-4, where 0 = next is parry 1)
        uint32_t GetCurrentParryCount(RE::Actor* actor) const;

        // Play slow time effect for timed blocks
        void PlaySlowTimeEffect(uint32_t parryLevel);

        // Trigger Elden Counter if available
        void TriggerEldenCounter(RE::Actor* blocker, uint32_t parryLevel);

    private:
        BlockEffectsHandler() = default;
        BlockEffectsHandler(const BlockEffectsHandler&) = delete;
        BlockEffectsHandler(BlockEffectsHandler&&) = delete;

        struct TimedBlockState {
            uint32_t consecutiveCount = 0;  // 0-4 for parries 1-5
            std::chrono::steady_clock::time_point lastParryTime;
            bool isPerfectParryActive = false;

            // Calculate current timeout based on parry count
            float GetCurrentTimeout(float baseTimeout) const {
                // Base + increment for each parry
                // After parry 1: base + 1
                // After parry 2: base + 2
                // After parry 3: base + 3
                // After parry 4: base + 4
                return baseTimeout + static_cast<float>(consecutiveCount);
            }
        };

        std::unordered_map<RE::FormID, TimedBlockState> actorStates;
        mutable std::mutex statesMutex;  // Protects actorStates from race conditions

        // Determine what equipment the blocker is using
        BlockEquipmentType GetBlockEquipmentType(RE::Actor* blocker);

        // Get the weapon/shield node for spark positioning
        RE::NiAVObject* GetBlockEquipmentNode(RE::Actor* blocker, BlockEquipmentType equipType);

        // Play the spark effect
        void PlayBlockSpark(RE::Actor* blocker, BlockEquipmentType equipType, uint32_t parryLevel);

        // Play the appropriate parry sound
        void PlayBlockSound(RE::Actor* blocker, BlockEquipmentType equipType, uint32_t parryLevel);

        // Apply heavy stagger (perfect parry / guard break) with custom magnitude
        void ApplyStagger(RE::Actor* aggressor, RE::Actor* blocker, float magnitude);

        // Apply light stagger (parries 1-4) with custom magnitude
        void ApplyLightStagger(RE::Actor* aggressor, RE::Actor* blocker, float magnitude);

        // Core stagger function (copied from Valhalla/Elden Parry)
        void TriggerStagger(RE::Actor* causer, RE::Actor* reactor, float magnitude);
    };

}
