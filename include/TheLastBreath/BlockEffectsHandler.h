#pragma once
#include <unordered_map>

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
        void OnSuccessfulTimedBlock(RE::Actor* blocker);

        // Call this when timed block fails or regular block happens
        void OnTimedBlockFailed(RE::Actor* blocker);

        // Clear tracking for an actor
        void ClearActor(RE::Actor* actor);

    private:
        BlockEffectsHandler() = default;
        BlockEffectsHandler(const BlockEffectsHandler&) = delete;
        BlockEffectsHandler(BlockEffectsHandler&&) = delete;

        struct TimedBlockState {
            uint32_t consecutiveCount = 0;  // 0-3 for cycling through sounds 1-4
        };

        std::unordered_map<RE::FormID, TimedBlockState> actorStates;

        // Determine what equipment the blocker is using
        BlockEquipmentType GetBlockEquipmentType(RE::Actor* blocker);

        // Get the weapon/shield node for spark positioning
        RE::NiAVObject* GetBlockEquipmentNode(RE::Actor* blocker, BlockEquipmentType equipType);

        // Play the spark effect
        void PlayBlockSpark(RE::Actor* blocker, BlockEquipmentType equipType);

        // Play the appropriate parry sound
        void PlayBlockSound(RE::Actor* blocker, BlockEquipmentType equipType, uint32_t soundIndex);
    };

}