#include "SIGA/SlowMotion.h"
#include "SIGA/Config.h"

namespace SIGA {

    SlowMotionManager* SlowMotionManager::GetSingleton() {
        static SlowMotionManager singleton;
        return &singleton;
    }

    void SlowMotionManager::ApplySlowdown(RE::Actor* actor, SlowType type, float skillLevel) {
        if (!actor) {
            logger::warn("ApplySlowdown called with null actor");
            return;
        }

        auto formID = actor->GetFormID();
        auto& state = actorStates[formID];

        // Store original speed if this is first slowdown
        if (!IsActorSlowed(actor)) {
            state.originalSpeedMult = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeedMult);
            logger::debug("Stored original speed: {}", state.originalSpeedMult);
        }

        logger::debug("ApplySlowdown: type={}, skillLevel={}", static_cast<int>(type), skillLevel);

        // Update state
        switch (type) {
        case SlowType::Bow:
        case SlowType::Crossbow:
            state.bowSlowActive = true;
            break;
        case SlowType::CastLeft:
            state.castLeftActive = true;
            state.lastCastTime = std::chrono::steady_clock::now();
            break;
        case SlowType::CastRight:
            state.castRightActive = true;
            state.lastCastTime = std::chrono::steady_clock::now();
            break;
        }

        // Check for dual casting befdsre calculating multiplier
        SlowType typeToUse = type;
        if (state.castLeftActive && state.castRightActive) {
            state.dualCastActive = true;
            typeToUse = SlowType::DualCast;
            logger::debug("Dual casting detected!");
        }

        // Calculate multiplier
        float multiplier = CalculateSpeedMultiplier(skillLevel, typeToUse);
        logger::debug("Calculated multiplier: {}", multiplier);

        // Apply multipleir relative to original speed
        float newSpeed = state.originalSpeedMult * multiplier;
        actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kSpeedMult, newSpeed);
        logger::debug("Set SpeedMult to {} (original {} * {})", newSpeed, state.originalSpeedMult, multiplier);

        // Force refresh
        actor->AsActorValueOwner()->ModActorValue(RE::ActorValue::kCarryWeight, 0.01f);
        actor->AsActorValueOwner()->ModActorValue(RE::ActorValue::kCarryWeight, -0.01f);
    }

    void SlowMotionManager::RemoveSlowdown(RE::Actor* actor, SlowType type) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        auto it = actorStates.find(formID);
        if (it == actorStates.end()) return;

        auto& state = it->second;

        // Update state flags
        switch (type) {
        case SlowType::Bow:
        case SlowType::Crossbow:
            state.bowSlowActive = false;
            break;
        case SlowType::CastLeft:
            state.castLeftActive = false;
            break;
        case SlowType::CastRight:
            state.castRightActive = false;
            break;
        case SlowType::DualCast:
            state.dualCastActive = false;
            break;
        }

        // Reset dual cast if either hand stops
        if (!state.castLeftActive || !state.castRightActive) {
            state.dualCastActive = false;
        }

        // If no slowdowns active, restore original speed
        if (!IsActorSlowed(actor)) {
            actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kSpeedMult, state.originalSpeedMult);
            logger::debug("Restored original speed: {}", state.originalSpeedMult);
            actorStates.erase(it);
        }
    }

    void SlowMotionManager::ClearAllSlowdowns(RE::Actor* actor) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        auto it = actorStates.find(formID);
        if (it == actorStates.end()) return;

        // Restore original speed directly
        actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kSpeedMult, it->second.originalSpeedMult);
        logger::debug("Cleared all slowdowns for actor, restored speed to {}", it->second.originalSpeedMult);
        actorStates.erase(it);
    }

    void SlowMotionManager::ClearAll() {
        for (auto& [formID, state] : actorStates) {
            if (auto actor = RE::TESForm::LookupByID<RE::Actor>(formID)) {
                actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kSpeedMult, state.originalSpeedMult);
            }
        }
        actorStates.clear();
        logger::debug("Cleared all slowdowns for all actors");
    }

    bool SlowMotionManager::IsActorSlowed(RE::Actor* actor) {
        auto it = actorStates.find(actor->GetFormID());
        if (it == actorStates.end()) return false;

        auto& state = it->second;
        return state.bowSlowActive || state.castLeftActive ||
            state.castRightActive || state.dualCastActive;
    }

    float SlowMotionManager::CalculateSpeedMultiplier(float skillLevel, SlowType type) {
        auto config = Config::GetSingleton();

        // Determine tier (Novice/Apprentice/Expert/Master)
        int tier = 0;
        if (skillLevel <= 25) tier = 0;
        else if (skillLevel <= 50) tier = 1;
        else if (skillLevel <= 75) tier = 2;
        else tier = 3;

        // Get multiplier from config
        float mult = 1.0f;
        switch (type) {
        case SlowType::Bow:
            mult = config->bowMultipliers[tier];
            break;
        case SlowType::Crossbow:
            mult = config->crossbowMultipliers[tier];
            break;
        case SlowType::CastLeft:
        case SlowType::CastRight:
            mult = config->castMultipliers[tier];
            break;
        case SlowType::DualCast:
            mult = config->dualCastMultipliers[tier];
            break;
        }

        return mult;
    }

} 