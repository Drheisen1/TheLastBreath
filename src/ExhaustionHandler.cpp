#include "TheLastBreath/ExhaustionHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void ExhaustionHandler::Update() {
        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement) {
            if (!actorStates.empty()) {
                ClearAll();
            }
            return;
        }

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        auto formID = player->GetFormID();

        // OPTIMIZATION: Use try_emplace instead of operator[] to avoid double hash lookup
        auto [it, inserted] = actorStates.try_emplace(formID);
        auto& state = it->second;

        float currentStamina = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

        // Handle exhaustion debuffs
        if (config->enableExhaustionDebuff) {
            bool shouldBeExhausted = (currentStamina < config->exhaustionStaminaThreshold);

            if (shouldBeExhausted && !state.isExhausted) {
                ApplyExhaustion(player);
                state.isExhausted = true;
                logger::debug("Exhaustion applied - stamina: {:.1f} < threshold: {:.1f}",
                    currentStamina, config->exhaustionStaminaThreshold);
            }
            else if (!shouldBeExhausted && state.isExhausted) {
                RemoveExhaustion(player);
                state.isExhausted = false;
                logger::debug("Exhaustion removed - stamina: {:.1f} >= threshold: {:.1f}",
                    currentStamina, config->exhaustionStaminaThreshold);
            }
        }
    }

    void ExhaustionHandler::ApplyExhaustion(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        auto avOwner = actor->AsActorValueOwner();
        auto formID = actor->GetFormID();

        // Get state (should already exist from Update())
        auto it = actorStates.find(formID);
        if (it == actorStates.end()) {
            logger::error("ApplyExhaustion called but state doesn't exist!");
            return;
        }
        auto& state = it->second;

        // BUG FIX: Store ORIGINAL values before modification
        // This prevents corruption if actor gains other buffs while exhausted
        state.originalSpeed = avOwner->GetActorValue(RE::ActorValue::kSpeedMult);
        state.originalAttackDamage = avOwner->GetActorValue(RE::ActorValue::kAttackDamageMult);
        state.originalDamageResist = avOwner->GetActorValue(RE::ActorValue::kDamageResist);

        // Apply debuffs from stored originals
        float newSpeed = state.originalSpeed * (1.0f - config->exhaustionMovementSpeedDebuff);
        avOwner->SetActorValue(RE::ActorValue::kSpeedMult, newSpeed);

        float newDamage = state.originalAttackDamage * (1.0f - config->exhaustionAttackDamageDebuff);
        avOwner->SetActorValue(RE::ActorValue::kAttackDamageMult, newDamage);

        float resistPenalty = (config->exhaustionDamageReceivedMult - 1.0f) * 100.0f;
        avOwner->SetActorValue(RE::ActorValue::kDamageResist, state.originalDamageResist - resistPenalty);

        logger::trace("Applied exhaustion debuffs - Speed: {:.0f}%, AttackDmg: {:.0f}%, DmgTaken: {:.0f}%",
            (1.0f - config->exhaustionMovementSpeedDebuff) * 100.0f,
            (1.0f - config->exhaustionAttackDamageDebuff) * 100.0f,
            config->exhaustionDamageReceivedMult * 100.0f);
    }

    void ExhaustionHandler::RemoveExhaustion(RE::Actor* actor) {
        if (!actor) return;

        auto avOwner = actor->AsActorValueOwner();
        auto formID = actor->GetFormID();

        // Get state
        auto it = actorStates.find(formID);
        if (it == actorStates.end()) {
            logger::error("RemoveExhaustion called but state doesn't exist!");
            return;
        }
        auto& state = it->second;

        // BUG FIX: Restore ORIGINAL stored values instead of calculating
        // This prevents corruption if actor gained other buffs while exhausted
        avOwner->SetActorValue(RE::ActorValue::kSpeedMult, state.originalSpeed);
        avOwner->SetActorValue(RE::ActorValue::kAttackDamageMult, state.originalAttackDamage);
        avOwner->SetActorValue(RE::ActorValue::kDamageResist, state.originalDamageResist);

        logger::trace("Removed exhaustion debuffs - restored original values");
    }

    void ExhaustionHandler::ClearAll() {
        for (auto& [formID, state] : actorStates) {
            if (state.isExhausted) {
                if (auto actor = RE::TESForm::LookupByID<RE::Actor>(formID)) {
                    RemoveExhaustion(actor);
                }
            }
        }
        actorStates.clear();
        logger::debug("Cleared all exhaustion states");
    }

}