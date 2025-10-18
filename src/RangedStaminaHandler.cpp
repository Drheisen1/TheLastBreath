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
        auto& state = actorStates[formID];

        // Only initialize if not already drawn
        if (!state.isDrawn) {
            state.isDrawn = true;
            state.drawStartTime = std::chrono::steady_clock::now();
            state.lastDrainTime = state.drawStartTime - std::chrono::milliseconds(200);  // Immediate first drain
            state.handle = actor->CreateRefHandle();
            logger::debug("Ranged weapon drawn - continuous stamina drain begins");
        }
        else {
            logger::debug("Ranged weapon already marked as drawn");
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
        if (!isRanged) {
            return;
        }

        // Apply flat release cost immediately (independent of draw state)
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

        // If we were tracking a drawn state, clear it now
        const auto formID = actor->GetFormID();
        if (auto it = actorStates.find(formID); it != actorStates.end()) {
            actorStates.erase(it);
        }
    }

    void RangedStaminaHandler::OnBlockStart(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement || !config->enableBlockStaminaDrain) {
            return;
        }

        auto formID = actor->GetFormID();
        auto& state = actorStates[formID];

        if (!state.isBlocking) {
            state.isBlocking = true;
            state.blockStartTime = std::chrono::steady_clock::now();
            state.lastBlockDrainTime = state.blockStartTime - std::chrono::milliseconds(200);  // Immediate first drain
            state.handle = actor->CreateRefHandle();
            logger::debug("Block started - continuous stamina drain begins");
        }
    }

    void RangedStaminaHandler::OnBlockStop(RE::Actor* actor) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        auto it = actorStates.find(formID);
        if (it != actorStates.end()) {
            it->second.isBlocking = false;
            logger::debug("Block stopped - stamina drain ends");

            // If not drawing bow either, clear the actor entirely
            if (!it->second.isDrawn) {
                actorStates.erase(it);
            }
        }
    }

    void RangedStaminaHandler::Update() {
        auto config = Config::GetSingleton();
        if (!config->enableStaminaManagement) return;

        auto now = std::chrono::steady_clock::now();

        for (auto it = actorStates.begin(); it != actorStates.end();) {
            auto& [formID, state] = *it;

            // Get the actor
            RE::Actor* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
            if (!actor) {
                it = actorStates.erase(it);
                continue;
            }

            // BOW DRAIN 
            if (state.isDrawn) {
                if (!config->enableRangedStaminaCost || !config->enableRangedHoldStaminaDrain) {
                    state.isDrawn = false;
                    if (!state.isBlocking) {
                        it = actorStates.erase(it);
                        continue;
                    }
                }
                else {
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
                        logger::debug("Bow draw interrupted - clearing tracking (IsAttacking: {}, HasBow: {})",
                            isStillDrawing, hasBowEquipped);
                        state.isDrawn = false;
                        if (!state.isBlocking) {
                            it = actorStates.erase(it);
                            continue;
                        }
                    }
                    else {
                        // Calculate time since last drain
                        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now - state.lastDrainTime).count();

                        if (elapsed >= 200) {
                            const float current = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
                            if (current <= 0.1f) {
                                logger::debug("Stamina exhausted - forcing bow state change");
                                actor->SetGraphVariableBool("IsAttacking", false);
                                actor->NotifyAnimationGraph("attackStop");
                                state.isDrawn = false;
                                if (!state.isBlocking) {
                                    it = actorStates.erase(it);
                                    continue;
                                }
                            }
                            else {
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
                        }
                    }
                }
            }

            // BLOCK DRAIN - Only process if blocking
            if (state.isBlocking) {
                if (!config->enableBlockStaminaDrain) {
                    state.isBlocking = false;
                    if (!state.isDrawn) {
                        it = actorStates.erase(it);
                        continue;
                    }
                }
                else {
                    // Validate block is still active
                    bool isStillBlocking = false;
                    actor->GetGraphVariableBool("IsBlocking", isStillBlocking);

                    if (!isStillBlocking) {
                        logger::debug("Block interrupted - clearing tracking");
                        state.isBlocking = false;
                        if (!state.isDrawn) {
                            it = actorStates.erase(it);
                            continue;
                        }
                    }
                    else {
                        // Calculate time since last block drain
                        auto blockElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now - state.lastBlockDrainTime).count();

                        if (blockElapsed >= 200) {
                            const float current = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
                            if (current <= 0.1f) {
                                logger::debug("Stamina exhausted - forcing block stop");
                                actor->SetGraphVariableBool("IsBlocking", false);
                                actor->NotifyAnimationGraph("blockStop");
                                state.isBlocking = false;
                                if (!state.isDrawn) {
                                    it = actorStates.erase(it);
                                    continue;
                                }
                            }
                            else {
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
                        }
                    }
                }
            }

            ++it;
        }
    }

    bool RangedStaminaHandler::IsActorTracked(RE::Actor* actor) const {
        if (!actor) return false;
        return actorStates.find(actor->GetFormID()) != actorStates.end();
    }

    void RangedStaminaHandler::ClearActor(RE::Actor* actor) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        auto it = actorStates.find(formID);
        if (it != actorStates.end()) {
            logger::debug("Clearing ranged stamina tracking for actor");
            actorStates.erase(it);
        }
    }

}
