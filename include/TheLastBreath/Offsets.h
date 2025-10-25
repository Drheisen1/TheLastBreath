#pragma once

namespace TheLastBreath {
    namespace Offsets {

        // Valhalla's PlaceAtMe wrapper
        inline RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* self, RE::TESForm* a_form, std::uint32_t count, bool forcePersist, bool initiallyDisabled)
        {
            using func_t = RE::TESObjectREFR* (RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*, RE::TESForm*, std::uint32_t, bool, bool);
            RE::VMStackID frame = 0;

            REL::Relocation<func_t> func{ RELOCATION_ID(55672, 56203) };
            auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();

            return func(vm, frame, self, a_form, count, forcePersist, initiallyDisabled);
        }

        // PushActorAway (for guard break / perfect parry)
        typedef void(_fastcall* tPushActorAway)(RE::AIProcess* a_causer, RE::Actor* a_target, RE::NiPoint3& a_origin, float a_magnitude);
        inline static REL::Relocation<tPushActorAway> PushActorAway{ RELOCATION_ID(38858, 39895) };

        // Apply stagger effect without damage
        typedef void(_fastcall* tStaggerActor)(RE::Actor* a_target, RE::Actor* a_aggressor, float a_magnitude);
        inline static REL::Relocation<tStaggerActor> StaggerActor{ RELOCATION_ID(36700, 37710) };

        // Set BSTimer function
        inline void SGTM(float a_multiplier, bool a_useSmoothing = true) {
            // Access BSTimer singleton directly
            REL::Relocation<RE::BSTimer**> singleton{ RELOCATION_ID(523657, 410196) };
            auto timer = *singleton;

            if (timer) {
                // Manually define function signature: void(BSTimer*, float, bool)
                using func_t = void(RE::BSTimer*, float, bool);
                REL::Relocation<func_t> func{ RELOCATION_ID(66988, 68245) };
                func(timer, a_multiplier, a_useSmoothing);
            }
        }

    } 
} 