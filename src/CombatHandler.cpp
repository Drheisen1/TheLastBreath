#include "TheLastBreath/CombatHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void CombatHandler::OnBlockStart(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement || !config->enableBlockStaminaDrain) {
            return;
        }

        // ============================================
        // CHECK: Only apply block drain for shields/weapons, NOT bows
        // ============================================
        bool hasBowEquipped = false;
        if (const auto obj = actor->GetEquippedObject(false)) {
            if (const auto weap = obj->As<RE::TESObjectWEAP>()) {
                const auto wt = weap->GetWeaponType();
                hasBowEquipped = (wt == RE::WEAPON_TYPE::kBow || wt == RE::WEAPON_TYPE::kCrossbow);
            }
        }

        if (hasBowEquipped) {
            logger::debug("Block button pressed but bow equipped - no stamina drain");
            return;
        }

        auto formID = actor->GetFormID();
        auto& state = actorBlockStates[formID];

        if (!state.isBlocking) {
            state.isBlocking = true;
            state.blockStartTime = std::chrono::steady_clock::now();
            state.lastBlockDrainTime = state.blockStartTime - std::chrono::milliseconds(200);
            logger::debug("Block started - continuous stamina drain begins");
        }
    }

    void CombatHandler::OnBlockStop(RE::Actor* actor) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        auto it = actorBlockStates.find(formID);
        if (it != actorBlockStates.end()) {
            logger::debug("Block stopped - stamina drain ends");
            actorBlockStates.erase(it);
        }
    }

    void CombatHandler::Update() {
        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement || !config->enableBlockStaminaDrain) {
            return;
        }

        auto now = std::chrono::steady_clock::now();

        // Handle block stamina drain
        for (auto it = actorBlockStates.begin(); it != actorBlockStates.end();) {
            auto& [formID, state] = *it;

            if (!state.isBlocking) {
                it = actorBlockStates.erase(it);
                continue;
            }

            RE::Actor* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
            if (!actor) {
                it = actorBlockStates.erase(it);
                continue;
            }

            // ============================================
            // SAFETY CHECK: Stop drain if bow is now equipped
            // ============================================
            bool hasBowEquipped = false;
            if (const auto obj = actor->GetEquippedObject(false)) {
                if (const auto weap = obj->As<RE::TESObjectWEAP>()) {
                    const auto wt = weap->GetWeaponType();
                    hasBowEquipped = (wt == RE::WEAPON_TYPE::kBow || wt == RE::WEAPON_TYPE::kCrossbow);
                }
            }

            if (hasBowEquipped) {
                logger::debug("Bow equipped during block drain - stopping");
                it = actorBlockStates.erase(it);
                continue;
            }

            auto blockElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - state.lastBlockDrainTime).count();

            if (blockElapsed >= 200) {
                const float current = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
                if (current <= 0.1f) {
                    logger::debug("Stamina exhausted - stopping block drain");
                    it = actorBlockStates.erase(it);
                    continue;
                }

                const float secondsElapsed = static_cast<float>(blockElapsed) / 1000.0f;
                const float costThisTick = config->blockHoldStaminaCostPerSecond * secondsElapsed;
                const float actualCost = std::min(costThisTick, current);

                actor->AsActorValueOwner()->RestoreActorValue(
                    RE::ACTOR_VALUE_MODIFIER::kDamage,
                    RE::ActorValue::kStamina,
                    -actualCost);

                logger::debug("Block hold drain: {:.2f} stamina ({} ms since last)",
                    actualCost, static_cast<int>(blockElapsed));

                state.lastBlockDrainTime = now;
            }

            ++it;
        }
    }

    void CombatHandler::OnActorHit(RE::Actor* victim, RE::Actor* aggressor, float actualDamage, BlockType blockType) {
        if (!victim) return;

        auto config = Config::GetSingleton();

        if (!victim->IsPlayerRef()) return;

        // ============================================
        // TIMED BLOCK (Always runs if enabled, independent of stamina management)
        // ============================================
        if (blockType == BlockType::Timed && config->enableTimedBlocking) {
            logger::info("=== TIMED BLOCK SUCCESS ===");

            // Trigger visual/audio effects and stagger
            BlockEffectsHandler::GetSingleton()->OnSuccessfulTimedBlock(victim, aggressor);

            // ============================================
            // DAMAGE REDUCTION VIA HEAL-BACK (ACCURATE)
            // ============================================
            if (config->timedBlockDamageReduction > 0.0f && actualDamage > 0.0f) {
                float damageToHealBack = actualDamage * config->timedBlockDamageReduction;

                victim->AsActorValueOwner()->RestoreActorValue(
                    RE::ACTOR_VALUE_MODIFIER::kDamage,
                    RE::ActorValue::kHealth,
                    damageToHealBack);

                logger::info("Timed block damage reduction: {:.2f} HP healed back ({:.0f}% of {:.2f} damage)",
                    damageToHealBack,
                    config->timedBlockDamageReduction * 100.0f,
                    actualDamage);
            }

            // Stamina handling for timed blocks (only if stamina management enabled)
            if (config->enableStaminaManagement) {
                float maxStamina = victim->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kStamina);
                float baseLoss = (config->staminaLossBaseIntercept - (config->staminaLossScalingFactor * maxStamina))
                    + config->staminaLossFlatAddition;
                baseLoss = std::max(0.0f, baseLoss);

                if (config->timedBlockStaminaLoss) {
                    float regularBlockLoss = baseLoss * config->regularBlockStaminaMult;
                    float timedBlockLoss = regularBlockLoss * config->timedBlockStaminaAmountLossMult;

                    if (timedBlockLoss > 0.0f) {
                        victim->AsActorValueOwner()->RestoreActorValue(
                            RE::ACTOR_VALUE_MODIFIER::kDamage,
                            RE::ActorValue::kStamina,
                            -timedBlockLoss);
                        logger::info("Timed block stamina LOSS: {:.2f}", timedBlockLoss);
                    }
                }
                else if (config->timedBlockStaminaGain) {
                    float staminaGain = config->timedBlockStaminaAmountGain;
                    if (staminaGain > 0.0f) {
                        victim->AsActorValueOwner()->RestoreActorValue(
                            RE::ACTOR_VALUE_MODIFIER::kDamage,
                            RE::ActorValue::kStamina,
                            staminaGain);
                        logger::info("Timed block stamina GAIN: {:.2f}", staminaGain);
                    }
                }
            }

            return;
        }

        // ============================================
        // STAMINA LOSS ON HIT (Regular blocks and no blocks)
        // ============================================

        // Exit if stamina management is disabled
        if (!config->enableStaminaManagement) {
            return;
        }

        // Master toggle for stamina loss on hit (controls entire Combat section)
        if (!config->enableStaminaLossOnHit) {
            return;
        }

        // Calculate base stamina loss
        float maxStamina = victim->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kStamina);
        float baseLoss = (config->staminaLossBaseIntercept - (config->staminaLossScalingFactor * maxStamina))
            + config->staminaLossFlatAddition;
        baseLoss = std::max(0.0f, baseLoss);

        logger::debug("Base stamina loss: {:.2f} (maxStamina: {:.0f})", baseLoss, maxStamina);

        float finalLoss = 0.0f;

        // ============================================
        // REGULAR BLOCK
        // ============================================
        if (blockType == BlockType::Regular) {
            // Reset timed block counter since this was not a timed block
            BlockEffectsHandler::GetSingleton()->OnTimedBlockFailed(victim);

            // Sub-toggle: only lose stamina on regular block if enabled
            if (config->enableRegularBlockStaminaLossOnHit) {
                finalLoss = baseLoss * config->regularBlockStaminaMult;
                logger::debug("Regular block - stamina loss: {:.2f}", finalLoss);
            }
            else {
                logger::debug("Regular block - stamina loss disabled");
                return;
            }
        }
        // ============================================
        // NO BLOCK
        // ============================================
        else if (blockType == BlockType::None) {
            // Reset timed block counter since no block happened
            BlockEffectsHandler::GetSingleton()->OnTimedBlockFailed(victim);

            // No sub-toggle - always lose stamina when hit without blocking
            finalLoss = baseLoss;
            logger::debug("No block - stamina loss: {:.2f}", finalLoss);
        }

        // Apply stamina loss
        if (finalLoss > 0.0f) {
            victim->AsActorValueOwner()->RestoreActorValue(
                RE::ACTOR_VALUE_MODIFIER::kDamage,
                RE::ActorValue::kStamina,
                -finalLoss);
        }
    }

}