#include "TheLastBreath/EldenCounterCompat.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void EldenCounterCompat::Initialize() {
        auto config = Config::GetSingleton();

        if (!config->enableEldenCounter) {
            logger::info("Elden Counter integration disabled");
            isAvailable = false;
            return;
        }

        logger::info("Initializing Elden Counter integration...");

        auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            isAvailable = false;
            return;
        }

        auto ecFile = dataHandler->LookupModByName("EldenCounter.esp");
        if (!ecFile) {
            logger::warn("EldenCounter.esp not found");
            isAvailable = false;
            return;
        }

        logger::info("Found EldenCounter.esp");
        FindTriggerSpell(ecFile);

        if (triggerSpell) {
            logger::info("Elden Counter integration initialized");
            logger::info("  - Will trigger on: {}",
                config->eldenCounterOnlyPerfectParry ? "Perfect parry only" : "All timed blocks");
            isAvailable = true;
        }
        else {
            logger::error("Could not find EC trigger spell");
            isAvailable = false;
        }
    }

    void EldenCounterCompat::FindTriggerSpell(const RE::TESFile* ecFile) {
        if (!ecFile) return;

        bool isLight = ecFile->IsLight();
        const RE::FormID triggerFormID = 0x801;

        RE::FormID fullTriggerID;

        if (isLight) {
            auto lightIndex = ecFile->smallFileCompileIndex;
            fullTriggerID = 0xFE000000 | (lightIndex << 12) | (triggerFormID & 0xFFF);
        }
        else {
            fullTriggerID = (ecFile->compileIndex << 24) | triggerFormID;
        }

        triggerSpell = RE::TESForm::LookupByID<RE::SpellItem>(fullTriggerID);

        if (triggerSpell) {
            logger::info("  - Found trigger spell: {} (0x{:X})",
                triggerSpell->GetName(), fullTriggerID);
        }
        else {
            logger::error("  - Trigger spell not found (0x{:X})", fullTriggerID);
        }
    }

    void EldenCounterCompat::TriggerCounter(RE::Actor* actor, bool isPerfectParry) {
        if (!actor || !isAvailable || !triggerSpell) {
            return;
        }

        auto config = Config::GetSingleton();

        if (!actor->IsPlayerRef()) {
            return;
        }

        // Check config
        if (config->eldenCounterOnlyPerfectParry && !isPerfectParry) {
            logger::debug("EC: Skipping (not perfect parry)");
            return;
        }

        logger::info("=== ELDEN COUNTER TRIGGERED ===");
        logger::info("  - Perfect Parry: {}", isPerfectParry);

        // Just add the trigger spell - EC's DLL handles everything else
        actor->AddSpell(triggerSpell);
        logger::info("  - Applied trigger spell - Elden Counter's DLL will handle the rest");
    }

}