#pragma once

namespace TheLastBreath {

    class Data {
    public:
        static Data* GetSingleton() {
            static Data singleton;
            return &singleton;
        }

        // Block FX
        static inline RE::TESObjectACTI* BlockFX = nullptr;
        static inline RE::BGSExplosion* BlockSpark = nullptr;
        static inline RE::BGSExplosion* BlockSparkFlare = nullptr;
        static inline RE::BGSExplosion* BlockSparkRing = nullptr;

        // Sounds
        static inline RE::BGSSoundDescriptorForm* parryWeaponSound1 = nullptr;
        static inline RE::BGSSoundDescriptorForm* parryWeaponSound2 = nullptr;
        static inline RE::BGSSoundDescriptorForm* parryWeaponSound3 = nullptr;
        static inline RE::BGSSoundDescriptorForm* parryWeaponSound4 = nullptr;
        static inline RE::BGSSoundDescriptorForm* parryShieldSound1 = nullptr;
        static inline RE::BGSSoundDescriptorForm* parryShieldSound2 = nullptr;
        static inline RE::BGSSoundDescriptorForm* parryShieldSound3 = nullptr;
        static inline RE::BGSSoundDescriptorForm* parryShieldSound4 = nullptr;

        // NEW: Sounds - Parry 5 (Perfect Parry)
        static inline RE::BGSSoundDescriptorForm* parryWeaponSoundPerfect = nullptr;
        static inline RE::BGSSoundDescriptorForm* parryShieldSoundPerfect = nullptr;

        // Load all data from plugin
        static void LoadData();

    private:
        Data() = default;
        Data(const Data&) = delete;
        Data(Data&&) = delete;

        static void LoadBlockFX(RE::TESDataHandler* dataHandler);
        static void LoadSounds(RE::TESDataHandler* dataHandler);
    };

}