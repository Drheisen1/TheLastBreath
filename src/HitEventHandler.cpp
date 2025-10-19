#include "TheLastBreath/HitEventHandler.h"
#include "TheLastBreath/CombatHandler.h"
#include "TheLastBreath/TimedBlockHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    RE::BSEventNotifyControl HitEventHandler::ProcessEvent(
        const RE::TESHitEvent* a_event,
        RE::BSTEventSource<RE::TESHitEvent>* a_eventSource)
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto victim = a_event->target.get();
        auto aggressor = a_event->cause.get();

        if (!victim || !aggressor) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto victimActor = victim->As<RE::Actor>();
        auto aggressorActor = aggressor->As<RE::Actor>();

        if (!victimActor || !aggressorActor) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // Only process if victim is player
        if (!victimActor->IsPlayerRef()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // FILTER: Only weapon/projectile hits, NO spells
        bool isWeaponHit = false;

        // Check if hit came from a weapon (melee)
        if (a_event->source != 0) {
            auto sourceForm = RE::TESForm::LookupByID(a_event->source);
            if (sourceForm && sourceForm->Is(RE::FormType::Weapon)) {
                isWeaponHit = true;
            }
        }

        // Check if hit came from a projectile (arrow/bolt)
        if (!isWeaponHit && a_event->projectile != 0) {
            auto projectileForm = RE::TESForm::LookupByID(a_event->projectile);
            if (projectileForm && projectileForm->Is(RE::FormType::Projectile)) {
                isWeaponHit = true;
            }
        }

        // Ignore spell hits
        if (!isWeaponHit) {
            logger::debug("Ignoring non-weapon hit (likely spell)");
            return RE::BSEventNotifyControl::kContinue;
        }

        bool wasBlocked = a_event->flags.all(RE::TESHitEvent::Flag::kHitBlocked);

        // Determine block type
        BlockType blockType = BlockType::None;
        if (wasBlocked) {
            auto timedBlockHandler = TheLastBreath::TimedBlockHandler::GetSingleton();
            blockType = timedBlockHandler->CheckBlockType(victimActor);

            // Consume the window if it was a timed block
            if (blockType == BlockType::Timed) {
                timedBlockHandler->ConsumeTimedBlock(victimActor);
            }
        }

        logger::debug("Player hit by {} (block type: {})",
            aggressorActor->GetName(),
            blockType == BlockType::Timed ? "TIMED" :
            blockType == BlockType::Regular ? "REGULAR" : "NONE");

        // Call combat handler with block type info
        CombatHandler::GetSingleton()->OnActorHit(victimActor, aggressorActor, 0.0f, blockType);

        return RE::BSEventNotifyControl::kContinue;
    }

}