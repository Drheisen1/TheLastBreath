#include "TheLastBreath/Data.h"

namespace TheLastBreath {

    void Data::LoadData() {
        auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            logger::error("Failed to get TESDataHandler!");
            return;
        }

        LoadBlockFX(dataHandler);
        LoadSounds(dataHandler);
    }

    void Data::LoadBlockFX(RE::TESDataHandler* dataHandler) {
        const char* pluginName = "TheLastBreath.esp";

        // Load activator
        BlockFX = dataHandler->LookupForm<RE::TESObjectACTI>(0x80D, pluginName);
        if (!BlockFX) {
            logger::error("Failed to load BlockFX activator!");
        }

        // Load explosions
        BlockSpark = dataHandler->LookupForm<RE::BGSExplosion>(0x808, pluginName);
        BlockSparkFlare = dataHandler->LookupForm<RE::BGSExplosion>(0x809, pluginName);
        BlockSparkRing = dataHandler->LookupForm<RE::BGSExplosion>(0x80A, pluginName);

        if (!BlockSpark || !BlockSparkFlare || !BlockSparkRing) {
            logger::error("Failed to load block explosion effects!");
        }
        else {
            logger::info("Block FX loaded successfully");
        }
    }

    void Data::LoadSounds(RE::TESDataHandler* dataHandler) {
        const char* pluginName = "TheLastBreath.esp";

        // Parries 1-4
        parryWeaponSound1 = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x800, pluginName);
        parryWeaponSound2 = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x801, pluginName);
        parryWeaponSound3 = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x802, pluginName);
        parryWeaponSound4 = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x803, pluginName);
        parryShieldSound1 = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x804, pluginName);
        parryShieldSound2 = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x805, pluginName);
        parryShieldSound3 = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x806, pluginName);
        parryShieldSound4 = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x807, pluginName);

        // Parry 5 (Perfect Parry)
        parryWeaponSoundPerfect = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x817, pluginName);
        parryShieldSoundPerfect = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x818, pluginName);

        if (!parryWeaponSound1 || !parryShieldSound1) {
            logger::error("Failed to load parry sounds!");
        }
        else {
            logger::info("Parry sounds loaded successfully");
            if (parryWeaponSoundPerfect && parryShieldSoundPerfect) {
                logger::info("Perfect parry sounds loaded successfully");
            }
        }
    }
}  // namespace TheLastBreath