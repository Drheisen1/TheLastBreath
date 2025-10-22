#include "TheLastBreath/AnimationHandler.h"
#include "TheLastBreath/SlowMotion.h"
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
        StartCustomA,
        HKS_TriggerA,
        BeginCastLeft,
        BeginCastRight,
        CastStop,
        CastOKStop,
        InterruptCast,
        AttackStop,
        WeaponSheathe,
        WeaponDraw,
        JumpUp,
    };

    // OPTIMIZATION: Hash map for O(1) event lookup instead of O(n) string comparisons
    static const std::unordered_map<std::string_view, AnimEventType> EVENT_LOOKUP = {
        {"BowDrawn", AnimEventType::BowDrawn},
        {"BowRelease", AnimEventType::BowRelease},
        {"bowRelease", AnimEventType::BowRelease},
        {"StartCustomA", AnimEventType::StartCustomA},
        {"HKS_TriggerA", AnimEventType::HKS_TriggerA},
        {"BeginCastLeft", AnimEventType::BeginCastLeft},
        {"BeginCastRight", AnimEventType::BeginCastRight},
        {"CastStop", AnimEventType::CastStop},
        {"CastOKStop", AnimEventType::CastOKStop},
        {"InterruptCast", AnimEventType::InterruptCast},
        {"attackStop", AnimEventType::AttackStop},
        {"WeaponSheathe", AnimEventType::WeaponSheathe},
        {"weaponSheathe", AnimEventType::WeaponSheathe},
        {"weaponDraw", AnimEventType::WeaponDraw},
        {"WeaponDraw", AnimEventType::WeaponDraw},
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
        auto slowMgr = SlowMotionManager::GetSingleton();
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
            slowMgr->RemoveSlowdown(actor, SlowType::Bow);
            slowMgr->RemoveSlowdown(actor, SlowType::Crossbow);
            break;
        }

        case AnimEventType::StartCustomA:
        {
            bool isRanged = false;
            if (const auto obj = actor->GetEquippedObject(false)) {
                if (const auto weap = obj->As<RE::TESObjectWEAP>()) {
                    const auto wt = weap->GetWeaponType();
                    isRanged = (wt == RE::WEAPON_TYPE::kBow || wt == RE::WEAPON_TYPE::kCrossbow);
                }
            }
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

        case AnimEventType::BeginCastLeft:
            logger::debug("BeginCastLeft event");
            OnBeginCastLeft(actor);
            break;

        case AnimEventType::BeginCastRight:
            logger::debug("BeginCastRight event");
            OnBeginCastRight(actor);
            break;

        case AnimEventType::CastStop:
            logger::debug("CastStop event");
            OnCastRelease(actor);
            break;

        case AnimEventType::CastOKStop:
        case AnimEventType::InterruptCast:
            if (slowMgr->IsActorSlowed(actor)) {
                logger::debug("Cast interrupted: {}", eventName);
                OnCastRelease(actor);
            }
            break;

        case AnimEventType::AttackStop:
            if (slowMgr->IsActorSlowed(actor)) {
                logger::debug("attackStop while slowed - clearing slowdowns");
                slowMgr->ClearAllSlowdowns(actor);
            }
            break;

        case AnimEventType::WeaponSheathe:
        case AnimEventType::WeaponDraw:
            if (slowMgr->IsActorSlowed(actor)) {
                logger::debug("Weapon state changed - clearing slowdowns");
                slowMgr->ClearAllSlowdowns(actor);
            }
            break;

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

        // Periodic cleanup of inactive slowdown states
        static int cleanupCounter = 0;
        if (++cleanupCounter >= 100) {
            slowMgr->CleanupInactiveStates();
            cleanupCounter = 0;
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    void AnimationEventHandler::OnBowDrawn(RE::Actor* actor) {
        auto config = Config::GetSingleton();
        auto rangedHandler = RangedStaminaHandler::GetSingleton();

        rangedHandler->OnRangedDrawn(actor);

        float archerySkill = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kArchery);

        auto equippedObject = actor->GetEquippedObject(false);
        bool isCrossbow = false;

        if (equippedObject) {
            auto weapon = equippedObject->As<RE::TESObjectWEAP>();
            if (weapon) {
                isCrossbow = (weapon->GetWeaponType() == RE::WEAPON_TYPE::kCrossbow);
            }
        }

        SlowType type = isCrossbow ? SlowType::Crossbow : SlowType::Bow;

        if (isCrossbow && !config->enableCrossbowDebuff) {
            logger::debug("Crossbow debuff disabled in config");
            return;
        }
        if (!isCrossbow && !config->enableBowDebuff) {
            logger::debug("Bow debuff disabled in config");
            return;
        }

        logger::debug("Applying {} slowdown (skill: {})", isCrossbow ? "crossbow" : "bow", archerySkill);
        SlowMotionManager::GetSingleton()->ApplySlowdown(actor, type, archerySkill);
    }

    void AnimationEventHandler::OnBeginCastLeft(RE::Actor* actor) {
        auto config = Config::GetSingleton();
        if (!config->enableCastDebuff) {
            return;
        }

        auto leftSpell = actor->GetActorRuntimeData().selectedSpells[RE::Actor::SlotTypes::kLeftHand];
        if (!leftSpell) {
            logger::debug("No spell in left hand");
            return;
        }

        if (SpellModifiesSpeed(leftSpell)) {
            logger::debug("Left spell modifies speed - skipping slowdown");
            return;
        }

        float skillLevel = GetMagicSkillLevel(actor, leftSpell);
        logger::debug("Left hand: {} (skill: {})", leftSpell->GetName(), skillLevel);
        SlowMotionManager::GetSingleton()->ApplySlowdown(actor, SlowType::CastLeft, skillLevel);
    }

    void AnimationEventHandler::OnBeginCastRight(RE::Actor* actor) {
        auto config = Config::GetSingleton();
        if (!config->enableCastDebuff) {
            return;
        }

        auto rightSpell = actor->GetActorRuntimeData().selectedSpells[RE::Actor::SlotTypes::kRightHand];
        if (!rightSpell) {
            logger::debug("No spell in right hand");
            return;
        }

        if (SpellModifiesSpeed(rightSpell)) {
            logger::debug("Right spell modifies speed - skipping slowdown");
            return;
        }

        float skillLevel = GetMagicSkillLevel(actor, rightSpell);
        logger::debug("Right hand: {} (skill: {})", rightSpell->GetName(), skillLevel);
        SlowMotionManager::GetSingleton()->ApplySlowdown(actor, SlowType::CastRight, skillLevel);
    }

    void AnimationEventHandler::OnCastRelease(RE::Actor* actor) {
        auto slowMgr = SlowMotionManager::GetSingleton();
        slowMgr->RemoveSlowdown(actor, SlowType::CastLeft);
        slowMgr->RemoveSlowdown(actor, SlowType::CastRight);
        slowMgr->RemoveSlowdown(actor, SlowType::DualCast);
        logger::debug("Cast released, removed all casting slowdowns");
    }

    void AnimationEventHandler::OnAttackStop(RE::Actor* actor) {
        SlowMotionManager::GetSingleton()->ClearAllSlowdowns(actor);
    }

    float AnimationEventHandler::GetMagicSkillLevel(RE::Actor* actor, RE::MagicItem* spell) {
        if (!spell) return 0.0f;

        auto spellItem = spell->As<RE::SpellItem>();
        if (!spellItem) {
            logger::warn("Could not cast spell to SpellItem");
            return 0.0f;
        }

        auto avOwner = actor->AsActorValueOwner();
        auto school = spellItem->GetAssociatedSkill();

        if (school == RE::ActorValue::kNone) {
            // Average all magic schools
            float total = 0.0f;
            total += avOwner->GetActorValue(RE::ActorValue::kDestruction);
            total += avOwner->GetActorValue(RE::ActorValue::kRestoration);
            total += avOwner->GetActorValue(RE::ActorValue::kAlteration);
            total += avOwner->GetActorValue(RE::ActorValue::kConjuration);
            total += avOwner->GetActorValue(RE::ActorValue::kIllusion);
            return total * 0.2f;
        }

        return avOwner->GetActorValue(school);
    }

    bool AnimationEventHandler::SpellModifiesSpeed(RE::MagicItem* spell) {
        if (!spell) return false;

        auto spellItem = spell->As<RE::SpellItem>();
        if (!spellItem) return false;

        for (auto effect : spellItem->effects) {
            if (effect && effect->baseEffect) {
                if (effect->baseEffect->data.primaryAV == RE::ActorValue::kSpeedMult) {
                    return true;
                }
            }
        }

        return false;
    }


}
