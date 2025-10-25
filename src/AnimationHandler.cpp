#include "TheLastBreath/AnimationHandler.h"
#include "TheLastBreath/RangedStaminaHandler.h"
#include "TheLastBreath/TimedBlockHandler.h"
#include "TheLastBreath/Config.h"
#include "TheLastBreath/EldenCounterCompat.h"
#include <unordered_map>


namespace TheLastBreath {

    // OPTIMIZATION: Event type enum for fast switch instead of string comparisons
    enum class AnimEventType {
        Unknown,
        BowDrawn,
        BowRelease,
        HKS_TriggerA,
        JumpUp,
    };

    // OPTIMIZATION: Hash map for O(1) event lookup instead of O(n) string comparisons
    static const std::unordered_map<std::string_view, AnimEventType> EVENT_LOOKUP = {
        {"BowDrawn", AnimEventType::BowDrawn},
        {"BowRelease", AnimEventType::BowRelease},
        {"bowRelease", AnimEventType::BowRelease},
        {"HKS_TriggerA", AnimEventType::HKS_TriggerA},
        {"JumpUp", AnimEventType::JumpUp},
    };

    AnimationEventHandler* AnimationEventHandler::GetSingleton() {
        static AnimationEventHandler singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl AnimationEventHandler::ProcessEvent(
        const RE::BSAnimationGraphEvent* a_event,
        RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
    {

        if (!a_event || !a_event->holder) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto actor = const_cast<RE::Actor*>(a_event->holder->As<RE::Actor>());
        if (!actor) {
            return RE::BSEventNotifyControl::kContinue;
        }

        bool isPlayer = actor->IsPlayerRef();

        // Handle NPCs
        if (!isPlayer) {
            auto config = Config::GetSingleton();

            if (!config->applyToNPCs) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (!actor->IsInCombat()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            logger::trace("Processing NPC event: {}", actor->GetName());
        }

        std::string_view eventName = a_event->tag;

        // OPTIMIZATION: Single hash lookup instead of multiple string comparisons
        auto eventIt = EVENT_LOOKUP.find(eventName);
        if (eventIt == EVENT_LOOKUP.end()) {
            // Unknown event, ignore
            return RE::BSEventNotifyControl::kContinue;
        }

        logger::trace("Animation event: '{}' from {}", eventName, isPlayer ? "Player" : actor->GetName());

        // Cache singleton pointers (minor optimization)
        auto rangedHandler = RangedStaminaHandler::GetSingleton();
        auto config = Config::GetSingleton();

        // OPTIMIZATION: Switch on enum instead of string comparisons
        switch (eventIt->second) {
        case AnimEventType::BowDrawn:
            logger::debug("Bow drawn event");
            OnBowDrawn(actor);
            break;

        case AnimEventType::BowRelease:
        {
            // only process if we're actually tracking OR if bow is equipped
            bool hasBowEquipped = false;
            if (const auto obj = actor->GetEquippedObject(false)) {
                if (const auto weap = obj->As<RE::TESObjectWEAP>()) {
                    const auto wt = weap->GetWeaponType();
                    hasBowEquipped = (wt == RE::WEAPON_TYPE::kBow || wt == RE::WEAPON_TYPE::kCrossbow);
                }
            }

            if (!hasBowEquipped) {
                return RE::BSEventNotifyControl::kContinue;
            }

            logger::debug("Bow release event");

            rangedHandler->OnRangedRelease(actor);
            break;
        }

        case AnimEventType::HKS_TriggerA:
        {
            logger::debug("HKS_TriggerA event (Rapid Combo)");

            if (!config->enableStaminaManagement || !config->enableRangedStaminaCost) {
                return RE::BSEventNotifyControl::kContinue;
            }

            bool isRanged = false;
            if (const auto obj = actor->GetEquippedObject(false)) {
                if (const auto weap = obj->As<RE::TESObjectWEAP>()) {
                    const auto wt = weap->GetWeaponType();
                    isRanged = (wt == RE::WEAPON_TYPE::kBow || wt == RE::WEAPON_TYPE::kCrossbow);
                }
            }

            if (!isRanged) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (config->enableRapidComboStaminaCost) {
                const float cost = config->rapidComboStaminaCost;
                if (cost > 0.0f) {
                    actor->AsActorValueOwner()->RestoreActorValue(
                        RE::ACTOR_VALUE_MODIFIER::kDamage,
                        RE::ActorValue::kStamina,
                        -cost);
                    logger::debug("Rapid Combo stamina cost: {}", cost);
                }
            }
            break;
        }

        case AnimEventType::JumpUp:
            if (config->enableStaminaManagement && config->enableJumpStaminaCost) {
                const float cost = config->jumpStaminaCost;
                if (cost > 0.0f) {
                    actor->AsActorValueOwner()->RestoreActorValue(
                        RE::ACTOR_VALUE_MODIFIER::kDamage,
                        RE::ActorValue::kStamina,
                        -cost);
                    logger::debug("Jump stamina cost: {}", cost);
                }
            }
            break;

        default:
            break;
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    void AnimationEventHandler::OnBowDrawn(RE::Actor* actor) {
        auto rangedHandler = RangedStaminaHandler::GetSingleton();
        rangedHandler->OnRangedDrawn(actor);
    }

}
