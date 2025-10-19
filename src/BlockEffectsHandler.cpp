#include "TheLastBreath/BlockEffectsHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void BlockEffectsHandler::OnSuccessfulTimedBlock(RE::Actor* blocker) {
        if (!blocker) return;

        auto formID = blocker->GetFormID();
        auto& state = actorStates[formID];

        // Determine equipment type
        auto equipType = GetBlockEquipmentType(blocker);
        if (equipType == BlockEquipmentType::None) {
            logger::warn("Timed block succeeded but no valid blocking equipment found");
            return;
        }

        // Play spark effect
        PlayBlockSpark(blocker, equipType);

        // Play sound (sound index cycles 0-3 for sounds 1-4)
        PlayBlockSound(blocker, equipType, state.consecutiveCount);

        // Increment counter and wrap around at 4
        state.consecutiveCount = (state.consecutiveCount + 1) % 4;

        logger::debug("Timed block effects: equipment={}, sound index={}",
            equipType == BlockEquipmentType::Shield ? "Shield" : "Weapon",
            state.consecutiveCount);
    }

    void BlockEffectsHandler::OnTimedBlockFailed(RE::Actor* blocker) {
        if (!blocker) return;

        auto formID = blocker->GetFormID();
        auto it = actorStates.find(formID);
        if (it != actorStates.end()) {
            it->second.consecutiveCount = 0;  // Reset to sound 1
            logger::debug("Timed block failed - sound counter reset");
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

        // Check left hand first (shield/weapon)
        auto leftEquipped = blocker->GetEquippedObject(true);
        if (leftEquipped) {
            if (leftEquipped->IsArmor()) {
                return BlockEquipmentType::Shield;
            }
            if (leftEquipped->IsWeapon()) {
                return BlockEquipmentType::Weapon;
            }
        }

        // Check right hand
        auto rightEquipped = blocker->GetEquippedObject(false);
        if (rightEquipped && rightEquipped->IsWeapon()) {
            return BlockEquipmentType::Weapon;
        }

        return BlockEquipmentType::None;
    }

    RE::NiAVObject* BlockEffectsHandler::GetBlockEquipmentNode(RE::Actor* blocker, BlockEquipmentType equipType) {
        if (!blocker) return nullptr;

        auto currentBiped = blocker->GetCurrentBiped();
        if (!currentBiped) return nullptr;

        // Determine which biped slot to use
        RE::BIPED_OBJECT bipeSlot = RE::BIPED_OBJECT::kNone;

        auto leftEquipped = blocker->GetEquippedObject(true);
        auto rightEquipped = blocker->GetEquippedObject(false);

        if (equipType == BlockEquipmentType::Shield && leftEquipped && leftEquipped->IsArmor()) {
            bipeSlot = RE::BIPED_OBJECT::kShield;
        }
        else if (leftEquipped && leftEquipped->IsWeapon()) {
            // Left hand weapon - determine type
            auto weapon = leftEquipped->As<RE::TESObjectWEAP>();
            if (weapon) {
                switch (weapon->GetWeaponType()) {
                case RE::WEAPON_TYPE::kOneHandSword:
                    bipeSlot = RE::BIPED_OBJECT::kShield;  // Left hand slot
                    break;
                case RE::WEAPON_TYPE::kOneHandDagger:
                    bipeSlot = RE::BIPED_OBJECT::kShield;
                    break;
                case RE::WEAPON_TYPE::kOneHandAxe:
                    bipeSlot = RE::BIPED_OBJECT::kShield;
                    break;
                case RE::WEAPON_TYPE::kOneHandMace:
                    bipeSlot = RE::BIPED_OBJECT::kShield;
                    break;
                default:
                    break;
                }
            }
        }
        else if (rightEquipped && rightEquipped->IsWeapon()) {
            // Right hand weapon
            auto weapon = rightEquipped->As<RE::TESObjectWEAP>();
            if (weapon) {
                switch (weapon->GetWeaponType()) {
                case RE::WEAPON_TYPE::kOneHandSword:
                    bipeSlot = RE::BIPED_OBJECT::kOneHandSword;
                    break;
                case RE::WEAPON_TYPE::kOneHandDagger:
                    bipeSlot = RE::BIPED_OBJECT::kOneHandDagger;
                    break;
                case RE::WEAPON_TYPE::kOneHandAxe:
                    bipeSlot = RE::BIPED_OBJECT::kOneHandAxe;
                    break;
                case RE::WEAPON_TYPE::kOneHandMace:
                    bipeSlot = RE::BIPED_OBJECT::kOneHandMace;
                    break;
                case RE::WEAPON_TYPE::kTwoHandSword:
                case RE::WEAPON_TYPE::kTwoHandAxe:
                    bipeSlot = RE::BIPED_OBJECT::kTwoHandMelee;
                    break;
                default:
                    break;
                }
            }
        }

        if (bipeSlot == RE::BIPED_OBJECT::kNone) {
            return nullptr;
        }

        auto& bipeObj = currentBiped->objects[bipeSlot];
        return bipeObj.partClone.get();
    }

    void BlockEffectsHandler::PlayBlockSpark(RE::Actor* blocker, BlockEquipmentType equipType) {
        if (!blocker || !blocker->Get3D()) return;

        auto equipNode = GetBlockEquipmentNode(blocker, equipType);
        if (!equipNode) {
            logger::warn("Could not find equipment node for spark effect");
            return;
        }

        // Determine which mesh to use
        const char* modelName = (equipType == BlockEquipmentType::Shield)
            ? "ValhallaCombat\\impactShieldRoot.nif"
            : "ValhallaCombat\\impactWeaponRoot.nif";

        // Get spark position from equipment node
        RE::NiPoint3 sparkPos = equipNode->world.translate;

        // Spawn the particle effect
        RE::BSTempEffectParticle::Spawn(
            blocker->GetParentCell(),
            0.0f,
            modelName,
            equipNode->world.rotate,
            sparkPos,
            1.0f,
            7,  // Flags for particle system
            equipNode);

        logger::debug("Played block spark at position ({:.2f}, {:.2f}, {:.2f})",
            sparkPos.x, sparkPos.y, sparkPos.z);
    }

    void BlockEffectsHandler::PlayBlockSound(RE::Actor* blocker, BlockEquipmentType equipType, uint32_t soundIndex) {
        if (!blocker) return;

        auto config = Config::GetSingleton();
        RE::FormID soundFormID = 0;

        // Get the appropriate sound FormID
        if (equipType == BlockEquipmentType::Shield) {
            switch (soundIndex) {
            case 0: soundFormID = config->parryShieldSound1; break;
            case 1: soundFormID = config->parryShieldSound2; break;
            case 2: soundFormID = config->parryShieldSound3; break;
            case 3: soundFormID = config->parryShieldSound4; break;
            }
            logger::debug("Playing shield parry sound {}", soundIndex + 1);
        }
        else {
            switch (soundIndex) {
            case 0: soundFormID = config->parryWeaponSound1; break;
            case 1: soundFormID = config->parryWeaponSound2; break;
            case 2: soundFormID = config->parryWeaponSound3; break;
            case 3: soundFormID = config->parryWeaponSound4; break;
            }
            logger::debug("Playing weapon parry sound {}", soundIndex + 1);
        }

        if (soundFormID == 0) {
            logger::error("Sound FormID not found for index {}", soundIndex);
            return;
        }

        // Play the sound using FormID
        auto audioManager = RE::BSAudioManager::GetSingleton();
        if (audioManager) {
            audioManager->Play(soundFormID);
        }
    }
}