#include "TheLastBreath/CombatHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void CombatHandler::OnBlockStart(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement || !config->enableBlockStaminaDrain) {
            return;
        }

        auto formID = actor->GetFormID();
        auto& state = actorBlockStates[formID];

        if (!state.isBlocking) {
            state.isBlocking = true;
            state.blockStartTime = std::chrono::steady_clock::now();
            state.lastBlockDrainTime = state.blockStartTime - std::chrono::milliseconds(200);  // Immediate first drain
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

    void CombatHandler::ApplyTimedBlockDamageResistance(RE::Actor* victim) {
        if (!victim) return;

        auto config = Config::GetSingleton();
        if (config->timedBlockDamageReduction <= 0.0f) return;

        float resistAmount = config->timedBlockDamageReduction * 100.0f;

        victim->AsActorValueOwner()->ModActorValue(
            RE::ActorValue::kDamageResist,
            resistAmount);

        auto& state = actorsWithDamageResistance[victim->GetFormID()];
        state.resistAmount = resistAmount;
        state.applyTime = std::chrono::steady_clock::now();

        logger::debug("Applied {:.0f}% damage resistance for timed block", resistAmount);
    }

    void CombatHandler::Update() {
        auto config = Config::GetSingleton();
        auto now = std::chrono::steady_clock::now();

        // Handle damage resistance removal
        if (!actorsWithDamageResistance.empty()) {
            std::vector<RE::FormID> toRemove;

            for (auto& [formID, state] : actorsWithDamageResistance) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - state.applyTime).count();

                if (elapsed >= 100) {
                    auto actor = RE::TESForm::LookupByID<RE::Actor>(formID);
                    if (actor) {
                        actor->AsActorValueOwner()->ModActorValue(
                            RE::ActorValue::kDamageResist,
                            -state.resistAmount);
                        logger::debug("Removed {:.0f}% damage resistance", state.resistAmount);
                    }
                    toRemove.push_back(formID);
                }
            }

            for (auto formID : toRemove) {
                actorsWithDamageResistance.erase(formID);
            }
        }

        // Handle block stamina drain
        if (!config->enableStaminaManagement || !config->enableBlockStaminaDrain) {
            return;
        }

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

    void CombatHandler::OnActorHit(RE::Actor* victim, RE::Actor* aggressor, float damage, BlockType blockType) {
        if (!victim) return;

        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement) return;

        if (!victim->IsPlayerRef()) return;

        float maxStamina = victim->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kStamina);
        float baseLoss = (config->staminaLossBaseIntercept - (config->staminaLossScalingFactor * maxStamina))
            + config->staminaLossFlatAddition;
        baseLoss = std::max(0.0f, baseLoss);

        logger::debug("Base stamina loss: {:.2f} (maxStamina: {:.0f})", baseLoss, maxStamina);

        // Handle TIMED BLOCK
        if (blockType == BlockType::Timed && config->enableTimedBlocking) {
            logger::info("=== TIMED BLOCK SUCCESS ===");

            // Play spark and sound effects
            BlockEffectsHandler::GetSingleton()->OnSuccessfulTimedBlock(victim);

            ApplyTimedBlockDamageResistance(victim);

            if (config->timedBlockStaminaLoss) {
                float regularBlockLoss = baseLoss * config->regularBlockStaminaMult;
                float timedBlockLoss = regularBlockLoss * config->timedBlockStaminaAmountLossMult;

                if (timedBlockLoss > 0.0f) {
                    victim->AsActorValueOwner()->RestoreActorValue(
                        RE::ACTOR_VALUE_MODIFIER::kDamage,
                        RE::ActorValue::kStamina,
                        -timedBlockLoss);
                    logger::info("Timed block stamina LOSS: {:.2f} (regular: {:.2f} x {:.2f})",
                        timedBlockLoss, regularBlockLoss, config->timedBlockStaminaAmountLossMult);
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

            return;
        }

        // Handle REGULAR BLOCK
        float finalLoss = 0.0f;
        if (blockType == BlockType::Regular) {
            // Reset timed block counter since this was not a timed block
            BlockEffectsHandler::GetSingleton()->OnTimedBlockFailed(victim);

            if (config->enableRegularBlockStaminaLossOnHit) {
                finalLoss = baseLoss * config->regularBlockStaminaMult;
                logger::debug("Regular block - stamina loss: {:.2f}", finalLoss);
            }
            else {
                logger::debug("Regular block - stamina loss disabled");
                return;
            }
        }
        // Handle NO BLOCK
        else if (blockType == BlockType::None) {
            // Reset timed block counter since no block happened
            BlockEffectsHandler::GetSingleton()->OnTimedBlockFailed(victim);

            if (!config->enableStaminaLossOnHit) {
                logger::debug("No block - stamina loss disabled");
                return;
            }
            finalLoss = baseLoss;
            logger::debug("No block - stamina loss: {:.2f}", finalLoss);
        }

        if (finalLoss > 0.0f) {
            victim->AsActorValueOwner()->RestoreActorValue(
                RE::ACTOR_VALUE_MODIFIER::kDamage,
                RE::ActorValue::kStamina,
                -finalLoss);
        }
    }

}