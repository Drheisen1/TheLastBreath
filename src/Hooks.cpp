#include "TheLastBreath/Hooks.h"
#include "TheLastBreath/Config.h"
#include "TheLastBreath/HitProcessor.h"
#include "TheLastBreath/EldenCounterCompat.h"

namespace TheLastBreath {
    namespace Hooks {

        // ============================================
        // ATTACK STAMINA COST HOOK
        // ============================================
        
        // Recursion guard
        static thread_local bool g_inCalculation = false;

        static float GetAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* attackData);
        static inline REL::Relocation<decltype(GetAttackStaminaCost)> _GetAttackStaminaCost;

        // Helper function to calculate power attack cost
        static float CalculatePowerAttackCost(RE::Actor* actor, RE::BGSAttackData* attackData) {
            if (!actor || !attackData) {
                return 35.0f;
            }

            g_inCalculation = true;

            bool wasPowerAttack = attackData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack);
            attackData->data.flags.set(RE::AttackData::AttackFlag::kPowerAttack);

            float powerCost = _GetAttackStaminaCost(actor->AsActorValueOwner(), attackData);

            if (!wasPowerAttack) {
                attackData->data.flags.reset(RE::AttackData::AttackFlag::kPowerAttack);
            }

            g_inCalculation = false;

            logger::debug("Calculated current power attack cost: {}", powerCost);
            return powerCost > 0.0f ? powerCost : 35.0f;
        }

        static float GetAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* attackData) {
            auto config = Config::GetSingleton();

            if (!config->enableStaminaManagement || !config->enableLightAttackStamina) {
                return _GetAttackStaminaCost(avOwner, attackData);
            }

            if (g_inCalculation) {
                return _GetAttackStaminaCost(avOwner, attackData);
            }

            // Try to get the actor
            auto actor = skyrim_cast<RE::Actor*>(avOwner);
            if (!actor) {
                return _GetAttackStaminaCost(avOwner, attackData);
            }

            // ============================================
            // VALHALLA'S METHOD: Get attackData from HighProcessData
            // Don't trust the hook parameter - get it from actor's process
            // ============================================
            RE::BGSAttackData* actualAttackData = nullptr;

            if (actor->GetActorRuntimeData().currentProcess &&
                actor->GetActorRuntimeData().currentProcess->high) {

                auto highProcess = actor->GetActorRuntimeData().currentProcess->high;
                if (highProcess->attackData) {
                    actualAttackData = highProcess->attackData.get();
                }
            }

            // Fallback: If process doesn't have it, try hook parameter
            if (!actualAttackData) {
                actualAttackData = attackData;
            }

            // ============================================
            // CHECK ATTACK TYPE (Like Valhalla)
            // ============================================

            // If we have attackData, check what type it is
            if (actualAttackData) {
                // Bash - return vanilla cost
                if (actualAttackData->data.flags.any(RE::AttackData::AttackFlag::kBashAttack)) {
                    float vanillaCost = _GetAttackStaminaCost(avOwner, attackData);
                    logger::debug("Bash - vanilla cost: {}", vanillaCost);
                    return vanillaCost;
                }

                // Power Attack - return vanilla cost
                if (actualAttackData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack)) {
                    float vanillaCost = _GetAttackStaminaCost(avOwner, attackData);
                    logger::debug("Power attack - vanilla cost: {}", vanillaCost);
                    return vanillaCost;
                }
            }

            // ============================================
            // LIGHT ATTACK - Calculate as % of power attack
            // ============================================

            // Use actualAttackData (from process) for calculation
            float powerAttackCost = CalculatePowerAttackCost(actor, actualAttackData);
            float lightCost = powerAttackCost * config->lightAttackStaminaCostMult;

            logger::debug("Light attack cost: {} ({}% of power: {})",
                lightCost, config->lightAttackStaminaCostMult * 100.0f, powerAttackCost);

            return lightCost;
        }
        // ============================================
        // HIT PROCESSING HOOK
        // ============================================

        // Function signature for Character::ProcessHitEvent
        using ProcessHitEvent_t = void(RE::Character*, RE::HitData&);
        static inline REL::Relocation<ProcessHitEvent_t> _ProcessHitEvent;

        static void ProcessHitEventHook(RE::Character* a_this, RE::HitData& a_hitData) {
            logger::trace("ProcessHitEventHook: Called for {}", a_this ? a_this->GetName() : "nullptr");

            // Get aggressor from hitData
            RE::Actor* aggressor = nullptr;
            if (a_hitData.aggressor) {
                aggressor = a_hitData.aggressor.get().get();
            }

            // Process through HitProcessor BEFORE damage calculation
            if (aggressor && a_this) {
                // Cast Character to Actor (Character inherits from Actor, so this is safe)
                auto victim = static_cast<RE::Actor*>(a_this);

                logger::trace("ProcessHitEventHook: Calling HitProcessor");
                HitProcessor::GetSingleton()->ProcessHit(aggressor, victim, a_hitData);
            }

            // Call original function (damage gets calculated with modified hitData)
            _ProcessHitEvent(a_this, a_hitData);
        }

        // ============================================
        // INSTALL FUNCTIONS
        // ============================================

        void Install() {
            logger::info("Installing attack stamina cost hook...");

            // Allocate MORE trampoline space since we're installing multiple hooks
            SKSE::AllocTrampoline(128);  // Increased from 64 to 128

            auto& trampoline = SKSE::GetTrampoline();
            REL::Relocation<uintptr_t> hook{ RELOCATION_ID(37650, 38603) };

            std::uintptr_t offset = REL::Module::IsAE() ? 0x171 : 0x16E;

            _GetAttackStaminaCost = trampoline.write_call<5>(hook.address() + offset, GetAttackStaminaCost);

            logger::info("Attack stamina cost hook installed");
        }

        void InstallHitHook() {
            logger::info("Installing hit processing hook...");

            // Hook Character::ProcessHitEvent directly using detour
            // SE: 37673, AE: 38626
            REL::Relocation<std::uintptr_t> ProcessHitEventAddr{ RELOCATION_ID(37673, 38626) };

            logger::info("ProcessHitEvent function address: {:X}", ProcessHitEventAddr.address());

            // Use trampoline to detour the function
            auto& trampoline = SKSE::GetTrampoline();
            _ProcessHitEvent = trampoline.write_branch<5>(
                ProcessHitEventAddr.address(),
                reinterpret_cast<std::uintptr_t>(ProcessHitEventHook)
            );

            logger::info("Hit processing hook installed via function detour");
        }

    }
}
