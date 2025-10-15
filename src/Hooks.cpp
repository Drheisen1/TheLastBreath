#include "TheLastBreath/Hooks.h"

namespace TheLastBreath {
    namespace Hooks {

        // This hook intercepts the power attack stamina cost calculation
        class AttackStaminaCostHook {
        public:
            static void Install() {
                auto& trampoline = SKSE::GetTrampoline();

                // Hook the function that calculates attack stamina cost
                REL::Relocation<uintptr_t> hook{ RELOCATION_ID(37650, 38603) };

                // Use conditional offset based on runtime version
                std::uintptr_t offset;
                if (REL::Module::IsAE()) {
                    offset = 0x171;  // AE offset
                }
                else {
                    offset = 0x16E;  // SE offset
                }

                _GetAttackStaminaCost = trampoline.write_call<5>(
                    hook.address() + offset,
                    GetAttackStaminaCost);

                logger::info("Attack stamina cost hook installed");
            }

        private:
            static float GetAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* attackData) {
                // Call original function to get vanilla cost
                float vanillaCost = _GetAttackStaminaCost(avOwner, attackData);

                // Check if this is a power attack
                if (attackData && attackData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack)) {
                    // Power attack - use vanilla cost
                    logger::debug("Power attack stamina cost: {}", vanillaCost);
                    return vanillaCost;
                }
                else {
                    // Light attack - use half of vanilla power attack cost
                    float lightCost = vanillaCost * 0.5f;
                    logger::debug("Light attack stamina cost: {} (half of {})", lightCost, vanillaCost);
                    return lightCost;
                }
            }

            static inline REL::Relocation<decltype(GetAttackStaminaCost)> _GetAttackStaminaCost;
        };

        void Install() {
            logger::info("Installing hooks...");
            SKSE::AllocTrampoline(64);
            AttackStaminaCostHook::Install();
            logger::info("Hooks installed successfully");
        }

    }
}