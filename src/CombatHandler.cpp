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

        // OPTIMIZATION: Use try_emplace instead of operator[]
        auto [it, inserted] = actorBlockStates.try_emplace(formID);
        auto& state = it->second;

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
        if (!ShouldProcessHit(victim)) {
            return;
        }

        auto config = Config::GetSingleton();

        // ============================================
        // TIMED BLOCK (Always runs if enabled, independent of stamina management)
        // ============================================
        if (blockType == BlockType::Timed && config->enableTimedBlocking) {
            ProcessTimedBlock(victim, aggressor, actualDamage);
            return;
        }

        // ============================================
        // STAMINA LOSS ON HIT (Regular blocks and no blocks)
        // ============================================
        if (!config->enableStaminaManagement || !config->enableStaminaLossOnHit) {
            return;
        }

        // Calculate base stamina loss
        float baseLoss = CalculateBaseStaminaLoss(victim);

        if (blockType == BlockType::Regular) {
            ProcessRegularBlock(victim, baseLoss);
        }
        else if (blockType == BlockType::None) {
            ProcessUnblockedHit(victim, baseLoss);
        }
    }

    // REFACTORED HELPER METHODS

    bool CombatHandler::ShouldProcessHit(RE::Actor* victim) {
        if (!victim) return false;
        if (!victim->IsPlayerRef()) return false;
        return true;
    }

    float CombatHandler::CalculateBaseStaminaLoss(RE::Actor* victim) {
        auto config = Config::GetSingleton();

        float maxStamina = victim->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kStamina);
        float baseLoss = (config->staminaLossBaseIntercept - (config->staminaLossScalingFactor * maxStamina))
            + config->staminaLossFlatAddition;
        baseLoss = std::max(0.0f, baseLoss);

        logger::debug("Base stamina loss: {:.2f} (maxStamina: {:.0f})", baseLoss, maxStamina);
        return baseLoss;
    }

    void CombatHandler::ProcessTimedBlock(RE::Actor* victim, RE::Actor* aggressor, float actualDamage) {
        auto config = Config::GetSingleton();

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
            float baseLoss = CalculateBaseStaminaLoss(victim);

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
    }

    void CombatHandler::ProcessRegularBlock(RE::Actor* victim, float baseLoss) {
        auto config = Config::GetSingleton();

        // Clear timed block state AND reset counter
        TimedBlockHandler::GetSingleton()->ClearActor(victim);
        BlockEffectsHandler::GetSingleton()->OnTimedBlockFailed(victim);

        // Sub-toggle: only lose stamina on regular block if enabled
        if (!config->enableRegularBlockStaminaLossOnHit) {
            logger::debug("Regular block - stamina loss disabled");
            return;
        }

        float finalLoss = baseLoss * config->regularBlockStaminaMult;
        logger::debug("Regular block - stamina loss: {:.2f}", finalLoss);

        if (finalLoss > 0.0f) {
            victim->AsActorValueOwner()->RestoreActorValue(
                RE::ACTOR_VALUE_MODIFIER::kDamage,
                RE::ActorValue::kStamina,
                -finalLoss);
        }
    }

    void CombatHandler::ProcessUnblockedHit(RE::Actor* victim, float baseLoss) {

        // Reset timed block counter since no block happened
        BlockEffectsHandler::GetSingleton()->OnTimedBlockFailed(victim);

        // No sub-toggle - always lose stamina when hit without blocking
        logger::debug("No block - stamina loss: {:.2f}", baseLoss);

        if (baseLoss > 0.0f) {

            victim->AsActorValueOwner()->RestoreActorValue(
                RE::ACTOR_VALUE_MODIFIER::kDamage,
                RE::ActorValue::kStamina,
                -baseLoss);
        }
    }

}