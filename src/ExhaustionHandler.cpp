#include "TheLastBreath/ExhaustionHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void ExhaustionHandler::Update() {
        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement) {
            std::lock_guard<std::mutex> lock(statesMutex);
            if (!actorStates.empty()) {
                ClearAll();
            }
            return;
        }

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        auto formID = player->GetFormID();

        std::lock_guard<std::mutex> lock(statesMutex);

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

        // NOTE: Mutex already held by caller (Update())
        // Get state (should already exist from Update())
        auto it = actorStates.find(formID);
        if (it == actorStates.end()) {
            logger::error("ApplyExhaustion called but state doesn't exist!");
            return;
        }
        auto& state = it->second;

        // Store current values BEFORE modification
        float currentSpeed = avOwner->GetActorValue(RE::ActorValue::kSpeedMult);
        float currentAttackDamage = avOwner->GetActorValue(RE::ActorValue::kAttackDamageMult);

        // Calculate the CHANGE needed (negative = debuff)
        float speedDelta = currentSpeed * -config->exhaustionMovementSpeedDebuff;
        float attackDelta = currentAttackDamage * -config->exhaustionAttackDamageDebuff;

        // Store the deltas so we can reverse them exactly
        state.originalSpeed = speedDelta;
        state.originalAttackDamage = attackDelta;

        // Apply using RestoreActorValue (delta-based)
        avOwner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kSpeedMult, speedDelta);
        avOwner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kAttackDamageMult, attackDelta);

        logger::debug("Applied exhaustion debuffs - Speed delta: {:.1f}, AttackDmg delta: {:.1f}",
            speedDelta, attackDelta);
    }

    void ExhaustionHandler::RemoveExhaustion(RE::Actor* actor) {
        if (!actor) return;

        auto avOwner = actor->AsActorValueOwner();
        auto formID = actor->GetFormID();

        // NOTE: Mutex already held by caller (Update() or ClearAll())
        // Get state
        auto it = actorStates.find(formID);
        if (it == actorStates.end()) {
            logger::error("RemoveExhaustion called but state doesn't exist!");
            return;
        }
        auto& state = it->second;

        // Restore by reversing the stored deltas
        // Negate the deltas to reverse them
        avOwner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kSpeedMult, -state.originalSpeed);
        avOwner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kAttackDamageMult, -state.originalAttackDamage);

        logger::debug("Removed exhaustion debuffs - reversed deltas (Speed: {:.1f}, AttackDmg: {:.1f})",
            -state.originalSpeed, -state.originalAttackDamage);
    }

    void ExhaustionHandler::ClearAll() {
        // NOTE: Mutex already held by caller (Update())
        for (auto& [formID, state] : actorStates) {
            if (state.isExhausted) {
                if (auto actor = RE::TESForm::LookupByID<RE::Actor>(formID)) {
                    if (!actor->IsDisabled() && !actor->IsDeleted()) {
                        RemoveExhaustion(actor);
                    }
                }
            }
        }
        actorStates.clear();
        logger::debug("Cleared all exhaustion states");
    }

}
