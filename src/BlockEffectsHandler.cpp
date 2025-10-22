#include "TheLastBreath/BlockEffectsHandler.h"
#include "TheLastBreath/Config.h"
#include "TheLastBreath/Data.h"
#include "TheLastBreath/Offsets.h"
#include "TheLastBreath/SlowTimeUtils.h"
#include "TheLastBreath/EldenCounterCompat.h"

namespace TheLastBreath {

    uint32_t BlockEffectsHandler::GetCurrentParryCount(RE::Actor* actor) const {
        if (!actor) return 0;

        auto it = actorStates.find(actor->GetFormID());
        if (it == actorStates.end()) return 0;

        return it->second.consecutiveCount;
    }

    void BlockEffectsHandler::PlaySlowTimeEffect(uint32_t parryLevel) {
        auto config = Config::GetSingleton();

        // Check if slow time should be applied
        bool shouldApplySlowTime = false;

        if (config->slowTimeOnlyOnPerfectParry) {
            // Only apply on perfect parry (level 5)
            shouldApplySlowTime = (parryLevel == 5);
        }
        else {
            // Apply on all timed blocks
            shouldApplySlowTime = true;
        }

        if (!shouldApplySlowTime) {
            return;
        }

        // Apply the slow time effect
        SlowTimeUtils::ApplySlowTime(config->slowTimeDuration, config->slowTimePercentage);

        if (parryLevel == 5) {
            logger::info("Applied PERFECT PARRY slow time effect");
        }
        else {
            logger::debug("Applied timed block slow time effect (parry {})", parryLevel);
        }
    }

    void BlockEffectsHandler::TriggerEldenCounter(RE::Actor* blocker, uint32_t parryLevel) {
        if (!blocker) return;

        auto config = Config::GetSingleton();
        if (!config->enableEldenCounter) return;

        auto eldenCounter = EldenCounterCompat::GetSingleton();
        if (!eldenCounter->IsAvailable()) return;

        bool isPerfectParry = (parryLevel == 5);
        eldenCounter->TriggerCounter(blocker, isPerfectParry);
    }

    void BlockEffectsHandler::OnSuccessfulTimedBlock(RE::Actor* blocker, RE::Actor* aggressor) {
        if (!blocker) return;

        auto config = Config::GetSingleton();
        auto formID = blocker->GetFormID();
        auto& state = actorStates[formID];

        // Update last parry time
        auto now = std::chrono::steady_clock::now();
        state.lastParryTime = now;

        // Determine equipment type
        auto equipType = GetBlockEquipmentType(blocker);
        if (equipType == BlockEquipmentType::None) {
            logger::warn("Timed block succeeded but no valid blocking equipment found");
            return;
        }

        // Determine parry level (1-5)
        uint32_t parryLevel = state.consecutiveCount + 1;

        // Check if we've reached perfect parry (parry 5)
        bool isPerfectParry = (parryLevel == 5 && config->enablePerfectParry);

        logger::info("=== PARRY {} {} ===",
            isPerfectParry ? "PERFECT" : std::to_string(parryLevel),
            equipType == BlockEquipmentType::Shield ? "(SHIELD)" : "(WEAPON)");

        // Play slow time effect
        PlaySlowTimeEffect(parryLevel);

        // Trigger Elden Counter (if enabled and available)
        auto eldenCounter = EldenCounterCompat::GetSingleton();
        eldenCounter->TriggerCounter(blocker, isPerfectParry);

        // Play spark effect
        if (config->enableParrySparks) {
            PlayBlockSpark(blocker, equipType, parryLevel);
        }

        // Play sound
        PlayBlockSound(blocker, equipType, parryLevel);
        // Apply stagger to aggressor
        if (aggressor && aggressor->Get3D()) {
            if (isPerfectParry) {
                // Perfect parry (5) - Heavy guard break stagger
                ApplyStagger(aggressor, blocker, config->perfectParryStaggerMagnitude);
                logger::info("Applied GUARD BREAK to {} (magnitude: {:.1f})",
                    aggressor->GetName(), config->perfectParryStaggerMagnitude);

                // Reset after perfect parry
                state.consecutiveCount = 0;
                state.isPerfectParryActive = false;
            }
            else if (config->enableParryStagger && parryLevel <= 4) {
                // Regular parry (1-4) - Escalating light stagger
                float magnitude = 0.0f;
                switch (parryLevel) {
                case 1: magnitude = config->parryStaggerMagnitude1; break;
                case 2: magnitude = config->parryStaggerMagnitude2; break;
                case 3: magnitude = config->parryStaggerMagnitude3; break;
                case 4: magnitude = config->parryStaggerMagnitude4; break;
                }

                ApplyLightStagger(aggressor, blocker, magnitude);

                // Calculate next timeout
                float nextTimeout = config->parrySequenceTimeoutBase + static_cast<float>(parryLevel);
                logger::debug("Applied parry {} stagger (magnitude: {:.1f}). Next timeout: {:.1f}s",
                    parryLevel, magnitude, nextTimeout);

                // Increment counter for next parry
                state.consecutiveCount++;
            }
        }
        else {
            // No aggressor - just increment or reset
            if (isPerfectParry) {
                state.consecutiveCount = 0;
                state.isPerfectParryActive = false;
            }
            else if (parryLevel < 5) {
                state.consecutiveCount++;
            }
        }

        logger::debug("Parry sequence: {}/5", state.consecutiveCount);
    }

    void BlockEffectsHandler::OnTimedBlockFailed(RE::Actor* blocker) {
        if (!blocker) return;

        auto formID = blocker->GetFormID();
        auto it = actorStates.find(formID);
        if (it != actorStates.end()) {
            it->second.consecutiveCount = 0;
            it->second.isPerfectParryActive = false;
            logger::debug("Parry sequence RESET - failed block");
        }
    }

    void BlockEffectsHandler::Update() {
        auto config = Config::GetSingleton();
        auto now = std::chrono::steady_clock::now();

        // Check for timeout on all active parry sequences
        for (auto it = actorStates.begin(); it != actorStates.end();) {
            auto& [formID, state] = *it;

            if (state.consecutiveCount > 0) {
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    now - state.lastParryTime).count();

                // Use dynamic timeout based on current parry count
                float currentTimeout = state.GetCurrentTimeout(config->parrySequenceTimeoutBase);

                if (elapsed >= currentTimeout) {
                    logger::debug("Parry sequence RESET - timeout ({} seconds, limit: {:.1f})",
                        elapsed, currentTimeout);
                    state.consecutiveCount = 0;
                    state.isPerfectParryActive = false;
                }
            }

            ++it;
        }
    }

    void BlockEffectsHandler::ClearActor(RE::Actor* actor) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        if (auto it = actorStates.find(formID); it != actorStates.end()) {
            actorStates.erase(it);
        }
    }

    BlockEquipmentType BlockEffectsHandler::GetBlockEquipmentType(RE::Actor* blocker) {
        if (!blocker) return BlockEquipmentType::None;

        auto leftEquipped = blocker->GetEquippedObject(true);
        if (leftEquipped) {
            if (leftEquipped->IsArmor()) {
                return BlockEquipmentType::Shield;
            }
            if (leftEquipped->IsWeapon()) {
                return BlockEquipmentType::Weapon;
            }
        }

        auto rightEquipped = blocker->GetEquippedObject(false);
        if (rightEquipped && rightEquipped->IsWeapon()) {
            return BlockEquipmentType::Weapon;
        }

        return BlockEquipmentType::None;
    }

    RE::NiAVObject* BlockEffectsHandler::GetBlockEquipmentNode(RE::Actor* blocker, BlockEquipmentType equipType) {
        if (!blocker) return nullptr;

        auto root3D = blocker->Get3D();
        if (!root3D) return nullptr;

        const char* nodeName = (equipType == BlockEquipmentType::Shield) ? "SHIELD" : "WEAPON";
        return root3D->GetObjectByName(nodeName);
    }

    void BlockEffectsHandler::PlayBlockSpark(RE::Actor* blocker, BlockEquipmentType equipType, uint32_t parryLevel) {
        if (!blocker || !blocker->Get3D()) {
            logger::error("PlayBlockSpark: Invalid blocker or missing 3D");
            return;
        }

        // Get the base activator
        auto activatorBase = Data::BlockFX;
        if (!activatorBase) {
            logger::error("BlockFX activator not loaded!");
            return;
        }

        // Spawn the activator at blocker's position
        auto blockFXNode = Offsets::PlaceAtMe(blocker, activatorBase, 1, false, false);
        if (!blockFXNode) {
            logger::error("Failed to spawn BlockFX activator!");
            return;
        }

        // Move to weapon/shield node
        std::string nodeName = (equipType == BlockEquipmentType::Shield) ? "SHIELD" : "WEAPON";
        blockFXNode->MoveToNode(blocker, nodeName);

        // Spawn explosions based on parry level
        if (Data::BlockSpark && Data::BlockSparkFlare) {
            Offsets::PlaceAtMe(blockFXNode, Data::BlockSpark, 1, false, false);
            Offsets::PlaceAtMe(blockFXNode, Data::BlockSparkFlare, 1, false, false);

            // Spawn ring explosion for perfect parry (parry 5)
            if (parryLevel == 5 && Data::BlockSparkRing) {
                Offsets::PlaceAtMe(blockFXNode, Data::BlockSparkRing, 1, false, false);
                logger::info("Spawned PERFECT PARRY spark with ring at {} node", nodeName);
            }
            else {
                logger::debug("Spawned parry {} spark at {} node", parryLevel, nodeName);
            }
        }

        // Immediately delete the activator
        blockFXNode->SetDelete(true);
    }

    void BlockEffectsHandler::PlayBlockSound(RE::Actor* blocker, BlockEquipmentType equipType, uint32_t parryLevel) {
        if (!blocker) return;

        RE::BGSSoundDescriptorForm* soundDescriptor = nullptr;

        // Get the appropriate sound based on parry level
        if (parryLevel == 5) {
            // Perfect parry sound
            soundDescriptor = (equipType == BlockEquipmentType::Shield)
                ? Data::parryShieldSoundPerfect
                : Data::parryWeaponSoundPerfect;
        }
        else {
            // Regular parry sounds (1-4)
            if (equipType == BlockEquipmentType::Shield) {
                switch (parryLevel) {
                case 1: soundDescriptor = Data::parryShieldSound1; break;
                case 2: soundDescriptor = Data::parryShieldSound2; break;
                case 3: soundDescriptor = Data::parryShieldSound3; break;
                case 4: soundDescriptor = Data::parryShieldSound4; break;
                }
            }
            else {
                switch (parryLevel) {
                case 1: soundDescriptor = Data::parryWeaponSound1; break;
                case 2: soundDescriptor = Data::parryWeaponSound2; break;
                case 3: soundDescriptor = Data::parryWeaponSound3; break;
                case 4: soundDescriptor = Data::parryWeaponSound4; break;
                }
            }
        }

        if (!soundDescriptor) {
            logger::error("Sound descriptor not found for parry level {}", parryLevel);
            return;
        }

        // Play the sound
        RE::BSSoundHandle soundHandle;
        soundHandle.soundID = static_cast<uint32_t>(-1);
        soundHandle.assumeSuccess = false;

        auto audioManager = RE::BSAudioManager::GetSingleton();
        if (audioManager) {
            bool success = audioManager->BuildSoundDataFromDescriptor(soundHandle, soundDescriptor);
            if (success && soundHandle.IsValid()) {
                soundHandle.SetPosition(blocker->GetPosition());
                soundHandle.SetObjectToFollow(blocker->Get3D());

                // Apply volume from config (0.0 - 1.0)
                auto config = Config::GetSingleton();
                if (config->parrySoundVolume >= 0.0f && config->parrySoundVolume <= 1.0f) {
                    soundHandle.SetVolume(config->parrySoundVolume);
                }

                soundHandle.Play();
                logger::debug("Playing {} parry {} sound (volume: {:.0f}%)",
                    equipType == BlockEquipmentType::Shield ? "shield" : "weapon",
                    parryLevel == 5 ? "PERFECT" : std::to_string(parryLevel),
                    config->parrySoundVolume * 100.0f);
            }
        }
    }

    void BlockEffectsHandler::ApplyLightStagger(RE::Actor* aggressor, RE::Actor* blocker, float magnitude) {
        if (!aggressor || !blocker) return;

        TriggerStagger(blocker, aggressor, magnitude);

        logger::debug("Applied light stagger ({:.1f}) to {}", magnitude, aggressor->GetName());
    }

    void BlockEffectsHandler::ApplyStagger(RE::Actor* aggressor, RE::Actor* blocker, float magnitude) {
        if (!aggressor || !blocker) return;

        TriggerStagger(blocker, aggressor, magnitude);

        logger::info("Applied GUARD BREAK stagger ({:.1f}) to {}", magnitude, aggressor->GetName());
    }

    void BlockEffectsHandler::TriggerStagger(RE::Actor* causer, RE::Actor* reactor, float magnitude) {
        if (!reactor || !causer) return;

        // Get causer's position
        RE::NiPoint3 causerPos = causer->GetPosition();

        // Calculate heading angle (false = allow negative angles)
        auto headingAngle = reactor->GetHeadingAngle(causerPos, false);

        // Convert to 0-1 range for stagger direction
        auto direction = (headingAngle >= 0.0f) ? headingAngle / 360.0f : (360.0f + headingAngle) / 360.0f;

        // Set graph variables
        reactor->SetGraphVariableFloat("staggerDirection", direction);
        reactor->SetGraphVariableFloat("StaggerMagnitude", magnitude);

        // Trigger stagger
        reactor->NotifyAnimationGraph("staggerStart");

        logger::trace("Triggered stagger on {} (magnitude: {:.1f}, direction: {:.2f})",
            reactor->GetName(), magnitude, direction);
    }
}
