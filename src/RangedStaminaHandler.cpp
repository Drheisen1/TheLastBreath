#include "TheLastBreath/RangedStaminaHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    RE::BSEventNotifyControl RangedStaminaHandler::ProcessEvent(
        const RE::MenuOpenCloseEvent* a_event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource)
    {
        Update();
        return RE::BSEventNotifyControl::kContinue;
    }

    void RangedStaminaHandler::OnRangedDrawn(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement || !config->enableRangedStaminaCost || !config->enableRangedHoldStaminaDrain) {
            return;
        }

        auto formID = actor->GetFormID();

        std::lock_guard<std::mutex> lock(statesMutex);

        // OPTIMIZATION: Use try_emplace instead of operator[]
        auto [it, inserted] = actorStates.try_emplace(formID);
        auto& state = it->second;

        if (!state.isDrawn) {
            state.isDrawn = true;
            state.drawStartTime = std::chrono::steady_clock::now();
            state.lastDrainTime = state.drawStartTime - std::chrono::milliseconds(200);
            state.handle = actor->CreateRefHandle();
            logger::debug("Ranged weapon drawn - continuous stamina drain begins");
        }
    }

    void RangedStaminaHandler::OnRangedRelease(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement || !config->enableRangedStaminaCost) return;

        bool isRanged = false;
        if (const auto obj = actor->GetEquippedObject(false)) {
            if (const auto weap = obj->As<RE::TESObjectWEAP>()) {
                const auto wt = weap->GetWeaponType();
                isRanged = (wt == RE::WEAPON_TYPE::kBow || wt == RE::WEAPON_TYPE::kCrossbow);
            }
        }
        if (!isRanged) return;

        if (config->enableRangedReleaseStaminaCost) {
            const float releaseCost = config->rangedReleaseStaminaCost;
            if (releaseCost > 0.0f) {
                actor->AsActorValueOwner()->RestoreActorValue(
                    RE::ACTOR_VALUE_MODIFIER::kDamage,
                    RE::ActorValue::kStamina,
                    -releaseCost);
                logger::debug("Ranged weapon release cost: {}", releaseCost);
            }
        }

        std::lock_guard<std::mutex> lock(statesMutex);

        const auto formID = actor->GetFormID();
        if (auto it = actorStates.find(formID); it != actorStates.end()) {
            actorStates.erase(it);
        }
    }

    void RangedStaminaHandler::Update() {
        auto config = Config::GetSingleton();

        if (!config->enableStaminaManagement || !config->enableRangedStaminaCost || !config->enableRangedHoldStaminaDrain) {
            return;
        }

        std::lock_guard<std::mutex> lock(statesMutex);

        auto now = std::chrono::steady_clock::now();

        for (auto it = actorStates.begin(); it != actorStates.end();) {
            auto& [formID, state] = *it;
            logger::trace("Processing actor FormID: {:X}, isDrawn: {}", formID, state.isDrawn);

            RE::Actor* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
            if (!actor || actor->IsDisabled() || actor->IsDeleted()) {
                it = actorStates.erase(it);
                continue;
            }

            if (!state.isDrawn) {
                it = actorStates.erase(it);
                continue;
            }

            // Validate bow is still drawn
            bool isStillDrawing = false;
            actor->GetGraphVariableBool("IsAttacking", isStillDrawing);

            bool hasBowEquipped = false;
            if (const auto obj = actor->GetEquippedObject(false)) {
                if (const auto weap = obj->As<RE::TESObjectWEAP>()) {
                    const auto wt = weap->GetWeaponType();
                    hasBowEquipped = (wt == RE::WEAPON_TYPE::kBow || wt == RE::WEAPON_TYPE::kCrossbow);
                }
            }

            if (!isStillDrawing || !hasBowEquipped) {
                logger::debug("Bow draw interrupted - clearing tracking");
                it = actorStates.erase(it);
                continue;
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - state.lastDrainTime).count();

            if (elapsed >= 200) {
                const float current = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
                if (current <= 0.1f) {
                    logger::debug("Stamina exhausted - forcing bow state change");
                    actor->SetGraphVariableBool("IsAttacking", false);
                    actor->NotifyAnimationGraph("attackStop");
                    it = actorStates.erase(it);
                    continue;
                }

                const float secondsElapsed = static_cast<float>(elapsed) / 1000.0f;
                const float costThisTick = config->rangedHoldStaminaCostPerSecond * secondsElapsed;
                const float actualCost = std::min(costThisTick, current);

                actor->AsActorValueOwner()->RestoreActorValue(
                    RE::ACTOR_VALUE_MODIFIER::kDamage,
                    RE::ActorValue::kStamina,
                    -actualCost);

                logger::debug("Ranged weapon hold drain: {:.2f} stamina ({} ms since last)",
                    actualCost, static_cast<int>(elapsed));

                state.lastDrainTime = now;
            }

            ++it;
        }
    }

    bool RangedStaminaHandler::IsActorTracked(RE::Actor* actor) const {
        if (!actor) return false;
        std::lock_guard<std::mutex> lock(statesMutex);
        return actorStates.find(actor->GetFormID()) != actorStates.end();
    }

    void RangedStaminaHandler::ClearActor(RE::Actor* actor) {
        if (!actor) return;
        std::lock_guard<std::mutex> lock(statesMutex);
        auto formID = actor->GetFormID();
        if (auto it = actorStates.find(formID); it != actorStates.end()) {
            logger::debug("Clearing ranged stamina tracking for actor");
            actorStates.erase(it);
        }
    }

}
