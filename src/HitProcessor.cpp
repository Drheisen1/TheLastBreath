#include "TheLastBreath/HitProcessor.h"
#include "TheLastBreath/Config.h"
#include "TheLastBreath/TimedBlockHandler.h"

namespace TheLastBreath {

    bool HitProcessor::ProcessHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData) {
        if (!aggressor || !victim) return false;

        auto config = Config::GetSingleton();
        if (!config->enableTimedBlocking) return false;

        // Only process for player (extend to NPCs later if desired)
        if (!victim->IsPlayerRef()) return false;

        // Check if this is a valid timed block
        if (IsValidTimedBlock(victim, aggressor, hitData)) {
            logger::info("=== TIMED BLOCK DETECTED (Hit Processor) ===");

            // Apply damage reduction to hit data BEFORE damage is calculated
            ApplyTimedBlockDamageReduction(hitData);

            return true;  // This was a timed block
        }

        return false;  // Not a timed block
    }

    bool HitProcessor::IsValidTimedBlock(RE::Actor* victim, RE::Actor* aggressor, const RE::HitData& hitData) {
        using HITFLAG = RE::HitData::Flag;

        // Must be blocking
        if (!hitData.flags.any(HITFLAG::kBlocked)) {
            return false;
        }

        // Check if timed block window is active
        auto timedBlockHandler = TimedBlockHandler::GetSingleton();
        if (!timedBlockHandler->IsTimedBlockWindowActive(victim)) {
            return false;
        }

        // Check if hit is from valid direction (front arc)
        // Skyrim already validates this via kBlocked flag, but add extra check
        auto victimAngle = victim->GetHeadingAngle(aggressor->GetPosition(), false);
        float angleDegrees = std::abs(victimAngle);

        // Front arc check: must be within ~120 degrees in front
        if (angleDegrees > 120.0f) {
            logger::debug("Timed block failed - hit from behind ({:.1f} degrees)", angleDegrees);
            return false;
        }

        logger::debug("Valid timed block - angle: {:.1f} degrees", angleDegrees);
        return true;
    }

    void HitProcessor::ApplyTimedBlockDamageReduction(RE::HitData& hitData) {
        auto config = Config::GetSingleton();

        // Get the damage reduction multiplier (0.0 to 1.0)
        float reductionMultiplier = config->timedBlockDamageReduction;

        // Clamp to valid range
        if (reductionMultiplier < 0.0f) reductionMultiplier = 0.0f;
        if (reductionMultiplier > 1.0f) reductionMultiplier = 1.0f;

        // Use percentBlocked to modify damage naturally
        // percentBlocked of 1.0 = 100% blocked = no damage
        // percentBlocked of 0.5 = 50% blocked = half damage

        float existingBlock = hitData.percentBlocked;
        float timedBlockBonus = reductionMultiplier * (1.0f - existingBlock);
        hitData.percentBlocked = existingBlock + timedBlockBonus;

        // Clamp to 1.0 (can't block more than 100%)
        if (hitData.percentBlocked > 1.0f) {
            hitData.percentBlocked = 1.0f;
        }

        logger::info("Applied timed block damage reduction: {:.0f}% blocked (was {:.0f}%, added {:.0f}%)",
            hitData.percentBlocked * 100.0f,
            existingBlock * 100.0f,
            timedBlockBonus * 100.0f);
    }

}