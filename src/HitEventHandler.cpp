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

        if (a_event->source != 0) {
            auto sourceForm = RE::TESForm::LookupByID(a_event->source);
            if (sourceForm && sourceForm->Is(RE::FormType::Weapon)) {
                isWeaponHit = true;
            }
        }

        if (!isWeaponHit && a_event->projectile != 0) {
            auto projectileForm = RE::TESForm::LookupByID(a_event->projectile);
            if (projectileForm && projectileForm->Is(RE::FormType::Projectile)) {
                isWeaponHit = true;
            }
        }

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

            if (blockType == BlockType::Timed) {
                timedBlockHandler->ConsumeTimedBlock(victimActor);
            }
        }

        // ============================================
        // GET ACTUAL DAMAGE FROM lastHitData
        // ============================================
        float actualDamageTaken = 0.0f;

        auto currentProcess = victimActor->GetActorRuntimeData().currentProcess;
        if (currentProcess && currentProcess->middleHigh) {
            auto lastHitData = currentProcess->middleHigh->lastHitData;
            if (lastHitData) {
                actualDamageTaken = lastHitData->totalDamage;

                logger::debug("Actual damage from lastHitData: {:.2f} (physical: {:.2f}, blocked: {:.1f}%)",
                    actualDamageTaken,
                    lastHitData->physicalDamage,
                    lastHitData->percentBlocked * 100.0f);
            }
        }

        logger::debug("Player hit by {} (block type: {}, damage: {:.2f})",
            aggressorActor->GetName(),
            blockType == BlockType::Timed ? "TIMED" :
            blockType == BlockType::Regular ? "REGULAR" : "NONE",
            actualDamageTaken);

        // Pass actual damage to combat handler
        CombatHandler::GetSingleton()->OnActorHit(victimActor, aggressorActor, actualDamageTaken, blockType);

        return RE::BSEventNotifyControl::kContinue;
    }

}