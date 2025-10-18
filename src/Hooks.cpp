#include "TheLastBreath/Hooks.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {
    namespace Hooks {

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
            auto config = TheLastBreath::Config::GetSingleton();

            if (!config->enableStaminaManagement || !config->enableLightAttackStamina) {
                return _GetAttackStaminaCost(avOwner, attackData);
            }

            if (g_inCalculation) {
                return _GetAttackStaminaCost(avOwner, attackData);
            }

            // Call original function to get vanilla cost
            float vanillaCost = _GetAttackStaminaCost(avOwner, attackData);

            // Try to get the actor
            auto actor = skyrim_cast<RE::Actor*>(avOwner);
            if (!actor) {
                logger::debug("Could not get actor, returning vanilla cost: {}", vanillaCost);
                return vanillaCost;
            }

            // Check if this is a power attack or bash
            bool isPowerAttack = (attackData && attackData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack));
            bool isBash = (attackData && attackData->data.flags.any(RE::AttackData::AttackFlag::kBashAttack));

            // Bashes use vanilla cost
            if (isBash) {
                logger::debug("Bash - using vanilla cost: {}", vanillaCost);
                return vanillaCost;
            }

            if (isPowerAttack) {
                // Power attack - return vanilla cost
                logger::debug("Power attack stamina cost: {}", vanillaCost);
                return vanillaCost;
            }
            else {
                // Light attack - calculate as % of power attack
                float powerAttackCost = CalculatePowerAttackCost(actor, attackData);
                float lightCost = powerAttackCost * config->lightAttackStaminaCostMult;

                logger::debug("Light attack stamina cost: {} ({}% of power attack: {})",
                    lightCost, config->lightAttackStaminaCostMult * 100.0f, powerAttackCost);
                return lightCost;
            }
        }

        void Install() {
            logger::info("Installing hooks...");

            SKSE::AllocTrampoline(64);

            auto& trampoline = SKSE::GetTrampoline();
            REL::Relocation<uintptr_t> hook{ RELOCATION_ID(37650, 38603) };

            std::uintptr_t offset = REL::Module::IsAE() ? 0x171 : 0x16E;

            _GetAttackStaminaCost = trampoline.write_call<5>(hook.address() + offset, GetAttackStaminaCost);

            logger::info("Attack stamina cost hook installed");
        }

    }
}
