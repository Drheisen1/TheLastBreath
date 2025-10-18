#include "TheLastBreath/CombatHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void CombatHandler::OnActorHit(RE::Actor* victim, RE::Actor* aggressor, float damage) {
        if (!victim) return;

        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement || !config->enableStaminaLossOnHit) return;

        // Only apply to player
        if (!victim->IsPlayerRef()) return;

        // Get armor skill (higher of light or heavy)
        float armorSkill = GetArmorSkill(victim);

        // Calculate stamina loss: Base * (100 - Skill) / 100
        float skillMultiplier = (100.0f - armorSkill) / 100.0f;
        float staminaLoss = config->staminaLossOnHitBase * skillMultiplier;

        if (staminaLoss > 0.0f) {
            auto formID = victim->GetFormID();
            auto now = std::chrono::steady_clock::now();

            // If there's already an active drain, add to it
            if (auto it = activeDrains.find(formID); it != activeDrains.end()) {
                it->second.totalAmount += staminaLoss;
                it->second.remaining += staminaLoss;
                logger::debug("Added {:.2f} to existing drain (total: {:.2f}, skill: {:.0f})",
                    staminaLoss, it->second.totalAmount, armorSkill);
            }
            else {
                // Create new gradual drain
                StaminaDrain drain;
                drain.totalAmount = staminaLoss;
                drain.remaining = staminaLoss;
                drain.startTime = now;
                drain.lastDrainTime = now;
                drain.duration = 3.0f;  // 3 seconds
                activeDrains[formID] = drain;

                logger::debug("Started gradual stamina drain: {:.2f} over {:.1f}s (skill: {:.0f})",
                    staminaLoss, drain.duration, armorSkill);
            }
        }
    }

    void CombatHandler::Update() {
        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement || !config->enableStaminaLossOnHit) {
            activeDrains.clear();
            return;
        }

        auto now = std::chrono::steady_clock::now();

        for (auto it = activeDrains.begin(); it != activeDrains.end();) {
            auto& [formID, drain] = *it;

            auto player = RE::PlayerCharacter::GetSingleton();
            if (!player || player->GetFormID() != formID) {
                it = activeDrains.erase(it);
                continue;
            }

            // Calculate time since last drain
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - drain.lastDrainTime).count();

            if (elapsed >= 100) {  // Drain every 100ms
                float secondsElapsed = static_cast<float>(elapsed) / 1000.0f;
                float drainRate = drain.totalAmount / drain.duration;  // stamina per second
                float costThisTick = drainRate * secondsElapsed;

                // Don't drain more than remaining
                costThisTick = std::min(costThisTick, drain.remaining);

                if (costThisTick > 0.0f) {
                    player->AsActorValueOwner()->RestoreActorValue(
                        RE::ACTOR_VALUE_MODIFIER::kDamage,
                        RE::ActorValue::kStamina,
                        -costThisTick);

                    drain.remaining -= costThisTick;
                    drain.lastDrainTime = now;

                    logger::debug("Gradual drain tick: {:.2f} stamina ({:.2f} remaining)",
                        costThisTick, drain.remaining);
                }

                // Remove if depleted
                if (drain.remaining <= 0.01f) {
                    logger::debug("Gradual drain completed");
                    it = activeDrains.erase(it);
                    continue;
                }
            }

            ++it;
        }
    }

    float CombatHandler::GetArmorSkill(RE::Actor* actor) {
        if (!actor) return 0.0f;

        auto avOwner = actor->AsActorValueOwner();

        float lightArmor = avOwner->GetActorValue(RE::ActorValue::kLightArmor);
        float heavyArmor = avOwner->GetActorValue(RE::ActorValue::kHeavyArmor);

        // Return the higher of the two skills
        return std::max(lightArmor, heavyArmor);
    }

}