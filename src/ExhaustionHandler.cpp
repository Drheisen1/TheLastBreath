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
        auto& state = actorStates[formID];

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

        // Apply movement speed debuff
        float currentSpeed = avOwner->GetActorValue(RE::ActorValue::kSpeedMult);
        float newSpeed = currentSpeed * (1.0f - config->exhaustionMovementSpeedDebuff);
        avOwner->SetActorValue(RE::ActorValue::kSpeedMult, newSpeed);

        // Apply attack damage debuff
        float currentDamage = avOwner->GetActorValue(RE::ActorValue::kAttackDamageMult);
        float newDamage = currentDamage * (1.0f - config->exhaustionAttackDamageDebuff);
        avOwner->SetActorValue(RE::ActorValue::kAttackDamageMult, newDamage);

        // Apply damage received multiplier
        avOwner->SetActorValue(RE::ActorValue::kDamageResist,
            avOwner->GetActorValue(RE::ActorValue::kDamageResist) - (config->exhaustionDamageReceivedMult - 1.0f) * 100.0f);

        logger::trace("Applied exhaustion debuffs - Speed: {:.0f}%, AttackDmg: {:.0f}%, DmgTaken: {:.0f}%",
            (1.0f - config->exhaustionMovementSpeedDebuff) * 100.0f,
            (1.0f - config->exhaustionAttackDamageDebuff) * 100.0f,
            config->exhaustionDamageReceivedMult * 100.0f);
    }

    void ExhaustionHandler::RemoveExhaustion(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        auto avOwner = actor->AsActorValueOwner();

        // Restore movement speed
        float currentSpeed = avOwner->GetActorValue(RE::ActorValue::kSpeedMult);
        float originalSpeed = currentSpeed / (1.0f - config->exhaustionMovementSpeedDebuff);
        avOwner->SetActorValue(RE::ActorValue::kSpeedMult, originalSpeed);

        // Restore attack damage
        float currentDamage = avOwner->GetActorValue(RE::ActorValue::kAttackDamageMult);
        float originalDamage = currentDamage / (1.0f - config->exhaustionAttackDamageDebuff);
        avOwner->SetActorValue(RE::ActorValue::kAttackDamageMult, originalDamage);

        // Restore damage resistance
        avOwner->SetActorValue(RE::ActorValue::kDamageResist,
            avOwner->GetActorValue(RE::ActorValue::kDamageResist) + (config->exhaustionDamageReceivedMult - 1.0f) * 100.0f);

        logger::trace("Removed exhaustion debuffs");
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