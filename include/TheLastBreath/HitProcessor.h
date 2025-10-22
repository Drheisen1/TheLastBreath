#pragma once

namespace TheLastBreath {

    class HitProcessor {
    public:
        static HitProcessor* GetSingleton() {
            static HitProcessor singleton;
            return &singleton;
        }

        // Process hit before damage is applied (called from hook)
        // Returns true if this was a timed block
        bool ProcessHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData);

    private:
        HitProcessor() = default;
        HitProcessor(const HitProcessor&) = delete;
        HitProcessor(HitProcessor&&) = delete;

        // Check if this hit should be a timed block
        bool IsValidTimedBlock(RE::Actor* victim, RE::Actor* aggressor, const RE::HitData& hitData);

        // Apply timed block damage reduction to hit data
        void ApplyTimedBlockDamageReduction(RE::HitData& hitData);
    };

}
