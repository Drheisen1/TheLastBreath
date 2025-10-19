#include "TheLastBreath/Config.h"
#include <SimpleIni.h>
#include <fstream>

namespace TheLastBreath {
    std::filesystem::path Config::GetConfigPath() {
        auto path = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "TheLastBreath.ini";
        return path;
    }

    void Config::Load() {
        CSimpleIniA ini;
        ini.SetUnicode();

        auto path = GetConfigPath();

        if (ini.LoadFile(path.string().c_str()) < 0) {
            logger::warn("Config file not found at {}, creating with defaults", path.string());
            Save();
            return;
        }

        // ===== STAMINA =====
        enableStaminaManagement = ini.GetBoolValue("Stamina", "bEnableStaminaManagement", true);
        enableJumpStaminaCost = ini.GetBoolValue("Stamina", "bEnableJumpStaminaCost", true);
        jumpStaminaCost = static_cast<float>(ini.GetDoubleValue("Stamina", "fJumpStaminaCost", 10.0));
        enableBlockStaminaDrain = ini.GetBoolValue("Stamina", "bEnableBlockStaminaDrain", true);                              // new
        blockHoldStaminaCostPerSecond = static_cast<float>(ini.GetDoubleValue("Stamina", "fBlockHoldStaminaCostPerSecond", 2.0));  // new

        // Stamina Loss on Hit
        enableStaminaLossOnHit = ini.GetBoolValue("Combat", "bEnableStaminaLossOnHit", true);
        staminaLossBaseIntercept = static_cast<float>(ini.GetDoubleValue("Combat", "fStaminaLossBaseIntercept", 14.5));
        staminaLossScalingFactor = static_cast<float>(ini.GetDoubleValue("Combat", "fStaminaLossScalingFactor", 0.018));
        staminaLossFlatAddition = static_cast<float>(ini.GetDoubleValue("Combat", "fStaminaLossFlatAddition", 1.0));
        enableRegularBlockStaminaLossOnHit = ini.GetBoolValue("Combat", "bEnableRegularBlockStaminaLossOnHit", true);
        regularBlockStaminaMult = static_cast<float>(ini.GetDoubleValue("Combat", "fRegularBlockStaminaMult", 0.5));


        // Melee Weapons
        enableLightAttackStamina = ini.GetBoolValue("Stamina", "bEnableLightAttackStamina", true);
        lightAttackStaminaCostMult = static_cast<float>(ini.GetDoubleValue("Stamina", "fLightAttackStaminaCost", 0.3));

        // Ranged Weapons
        enableRangedStaminaCost = ini.GetBoolValue("Stamina", "bEnableRangedStaminaCost", true);
        enableRangedHoldStaminaDrain = ini.GetBoolValue("Stamina", "bEnableRangedHoldStaminaDrain", true);
        enableRangedReleaseStaminaCost = ini.GetBoolValue("Stamina", "bEnableRangedReleaseStaminaCost", true);
        rangedHoldStaminaCostPerSecond = static_cast<float>(ini.GetDoubleValue("Stamina", "fRangedHoldStaminaCostPerSecond", 3.0));
        rangedReleaseStaminaCost = static_cast<float>(ini.GetDoubleValue("Stamina", "fRangedReleaseStaminaCost", 10.0));
        enableRapidComboStaminaCost = ini.GetBoolValue("Stamina", "bEnableRapidComboStaminaCost", false);              // new
        rapidComboStaminaCost = static_cast<float>(ini.GetDoubleValue("Stamina", "fRapidComboStaminaCost", 10.0));   // new

        // Exhaustion System
        enableExhaustionDebuff = ini.GetBoolValue("Exhaustion", "bEnableExhaustionDebuff", true); // new
        exhaustionStaminaThreshold = static_cast<float>(ini.GetDoubleValue("Exhaustion", "fExhaustionStaminaThreshold", 20.0)); // new
        exhaustionMovementSpeedDebuff = static_cast<float>(ini.GetDoubleValue("Exhaustion", "fExhaustionMovementSpeedDebuff", 0.20)); // new
        exhaustionAttackDamageDebuff = static_cast<float>(ini.GetDoubleValue("Exhaustion", "fExhaustionAttackDamageDebuff", 0.25)); // new
        exhaustionDamageReceivedMult = static_cast<float>(ini.GetDoubleValue("Exhaustion", "fExhaustionDamageReceivedMult", 1.25)); // new

        // Timed Blocking
        enableTimedBlocking = ini.GetBoolValue("TimedBlocking", "bEnableTimedBlocking", true);
        timedBlockWindow = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockWindow", 0.3));
        timedBlockAnimationDelay = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockAnimationDelay", 0.25));
        blockButton = static_cast<uint32_t>(ini.GetLongValue("TimedBlocking", "iBlockButton", 257));
        timedBlockStaminaLoss = ini.GetBoolValue("TimedBlocking", "bTimedBlockStaminaLoss", false);
        timedBlockStaminaGain = ini.GetBoolValue("TimedBlocking", "bTimedBlockStaminaGain", true);
        timedBlockStaminaAmountGain = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockStaminaAmountGain", 20.0));
        timedBlockStaminaAmountLossMult = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockStaminaAmountLossMult", 0.5));
        timedBlockDamageReduction = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockDamageReduction", 1.0));


        // ===== CASTING DEBUFF =====
        applyToNPCs = ini.GetBoolValue("CastingDebuff", "bApplyToNPCs", true);

        // Bow
        enableBowDebuff = ini.GetBoolValue("CastingDebuff.Bow", "bEnableBowDebuff", true);
        bowMultipliers[0] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Bow", "fNoviceMultiplier", 0.5));
        bowMultipliers[1] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Bow", "fApprenticeMultiplier", 0.6));
        bowMultipliers[2] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Bow", "fExpertMultiplier", 0.7));
        bowMultipliers[3] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Bow", "fMasterMultiplier", 0.8));

        // Crossbow
        enableCrossbowDebuff = ini.GetBoolValue("CastingDebuff.Crossbow", "bEnableCrossbowDebuff", true);
        crossbowMultipliers[0] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Crossbow", "fNoviceMultiplier", 0.5));
        crossbowMultipliers[1] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Crossbow", "fApprenticeMultiplier", 0.6));
        crossbowMultipliers[2] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Crossbow", "fExpertMultiplier", 0.7));
        crossbowMultipliers[3] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Crossbow", "fMasterMultiplier", 0.8));

        // Cast
        enableCastDebuff = ini.GetBoolValue("CastingDebuff.Cast", "bEnableCastDebuff", true);
        castMultipliers[0] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Cast", "fNoviceMultiplier", 0.5));
        castMultipliers[1] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Cast", "fApprenticeMultiplier", 0.6));
        castMultipliers[2] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Cast", "fExpertMultiplier", 0.7));
        castMultipliers[3] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.Cast", "fMasterMultiplier", 0.8));

        // Dual Cast
        enableDualCastDebuff = ini.GetBoolValue("CastingDebuff.DualCast", "bEnableDualCastDebuff", true);
        dualCastMultipliers[0] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.DualCast", "fNoviceMultiplier", 0.4));
        dualCastMultipliers[1] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.DualCast", "fApprenticeMultiplier", 0.5));
        dualCastMultipliers[2] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.DualCast", "fExpertMultiplier", 0.6));
        dualCastMultipliers[3] = static_cast<float>(ini.GetDoubleValue("CastingDebuff.DualCast", "fMasterMultiplier", 0.7));

        // ===== DEBUG =====
        logLevel = ini.GetLongValue("Debug", "iLogLevel", 1);

        logger::info("Config loaded successfully from {}", path.string());
    }

    void Config::Save() {
        CSimpleIniA ini;
        ini.SetUnicode();

        // ===== GENERAL =====
        ini.SetValue("General", nullptr, nullptr);
        ini.SetValue("General", nullptr, "; ============================================");
        ini.SetValue("General", nullptr, "; The Last Breath - Combat Overhaul");
        ini.SetValue("General", nullptr, "; ============================================");
        ini.SetValue("General", nullptr, nullptr);

        // ===== STAMINA =====
        ini.SetValue("Stamina", nullptr, nullptr);
        ini.SetValue("Stamina", nullptr, "; ============================================");
        ini.SetValue("Stamina", nullptr, "; STAMINA MANAGEMENT SYSTEM");
        ini.SetValue("Stamina", nullptr, "; ============================================");
        ini.SetValue("Stamina", nullptr, nullptr);

        ini.SetValue("Stamina", nullptr, "; Enable/disable all stamina management features");
        ini.SetBoolValue("Stamina", "bEnableStaminaManagement", enableStaminaManagement);

        ini.SetValue("Stamina", nullptr, nullptr);
        ini.SetValue("Stamina", nullptr, "; --- Jump Stamina Cost ---");
        ini.SetValue("Stamina", nullptr, "; Enable stamina cost when jumping");
        ini.SetBoolValue("Stamina", "bEnableJumpStaminaCost", enableJumpStaminaCost);
        ini.SetValue("Stamina", nullptr, "; Flat stamina cost per jump");
        ini.SetDoubleValue("Stamina", "fJumpStaminaCost", jumpStaminaCost);

        ini.SetValue("Stamina", nullptr, nullptr);                                                      // new
        ini.SetValue("Stamina", nullptr, "; --- Block Stamina Drain ---");                             // new
        ini.SetValue("Stamina", nullptr, "; Enable continuous stamina drain while blocking");          // new
        ini.SetBoolValue("Stamina", "bEnableBlockStaminaDrain", enableBlockStaminaDrain);              // new
        ini.SetValue("Stamina", nullptr, "; Stamina drain per second while blocking");                 // new
        ini.SetDoubleValue("Stamina", "fBlockHoldStaminaCostPerSecond", blockHoldStaminaCostPerSecond); // new

        ini.SetValue("Stamina", nullptr, nullptr);
        ini.SetValue("Stamina", nullptr, "; --- Light Attack Stamina Cost ---");
        ini.SetValue("Stamina", nullptr, "; Enable light attack stamina cost system");
        ini.SetBoolValue("Stamina", "bEnableLightAttackStamina", enableLightAttackStamina);
        ini.SetValue("Stamina", nullptr, "; Light attack stamina cost as % of power attack");
        ini.SetValue("Stamina", nullptr, "; 0.3 = 30% of power attack cost, 1.0 = same as power attack");
        ini.SetDoubleValue("Stamina", "fLightAttackStaminaCost", lightAttackStaminaCostMult);

        ini.SetValue("Stamina", nullptr, nullptr);
        ini.SetValue("Stamina", nullptr, "; --- Ranged Weapons (Bow & Crossbow) ---");
        ini.SetValue("Stamina", nullptr, "; Master toggle for ranged stamina costs");
        ini.SetBoolValue("Stamina", "bEnableRangedStaminaCost", enableRangedStaminaCost);

        ini.SetValue("Stamina", nullptr, nullptr);
        ini.SetValue("Stamina", nullptr, "; Enable continuous stamina drain while holding bow/crossbow drawn");
        ini.SetBoolValue("Stamina", "bEnableRangedHoldStaminaDrain", enableRangedHoldStaminaDrain);
        ini.SetValue("Stamina", nullptr, "; Stamina drain per second while aiming");
        ini.SetDoubleValue("Stamina", "fRangedHoldStaminaCostPerSecond", rangedHoldStaminaCostPerSecond);

        ini.SetValue("Stamina", nullptr, nullptr);
        ini.SetValue("Stamina", nullptr, "; Enable stamina cost when firing arrow");
        ini.SetBoolValue("Stamina", "bEnableRangedReleaseStaminaCost", enableRangedReleaseStaminaCost);
        ini.SetValue("Stamina", nullptr, "; Stamina cost when firing");
        ini.SetDoubleValue("Stamina", "fRangedReleaseStaminaCost", rangedReleaseStaminaCost);

        ini.SetValue("Stamina", nullptr, nullptr);                                                      // new
        ini.SetValue("Stamina", nullptr, "; --- Rapid Combo (Bow Rapid Combo V3 Mod) ---");            // new
        ini.SetValue("Stamina", nullptr, "; Enable stamina cost for rapid combo arrows");              // new
        ini.SetBoolValue("Stamina", "bEnableRapidComboStaminaCost", enableRapidComboStaminaCost);      // new
        ini.SetValue("Stamina", nullptr, "; Stamina cost per rapid combo arrow");                      // new
        ini.SetDoubleValue("Stamina", "fRapidComboStaminaCost", rapidComboStaminaCost);

        ini.SetValue("Combat", nullptr, nullptr);
        ini.SetValue("Combat", nullptr, "; ============================================");
        ini.SetValue("Combat", nullptr, "; COMBAT DAMAGE SYSTEM");
        ini.SetValue("Combat", nullptr, "; ============================================");
        ini.SetValue("Combat", nullptr, nullptr);

        ini.SetValue("Combat", nullptr, "; Enable stamina loss when player is hit (no block)");
        ini.SetBoolValue("Combat", "bEnableStaminaLossOnHit", enableStaminaLossOnHit);

        ini.SetValue("Combat", nullptr, nullptr);
        ini.SetValue("Combat", nullptr, "; Stamina loss formula: (BaseIntercept - (ScalingFactor * MaxStamina)) + FlatAddition");
        ini.SetValue("Combat", nullptr, "; Example: (14.5 - (0.018 * 100)) + 1 = 13.7 stamina loss at 100 max stamina");
        ini.SetValue("Combat", nullptr, "; Base stamina loss at 0 max stamina");
        ini.SetDoubleValue("Combat", "fStaminaLossBaseIntercept", staminaLossBaseIntercept);

        ini.SetValue("Combat", nullptr, "; How much to reduce per point of max stamina");
        ini.SetDoubleValue("Combat", "fStaminaLossScalingFactor", staminaLossScalingFactor);

        ini.SetValue("Combat", nullptr, "; Flat amount always added to the result");
        ini.SetDoubleValue("Combat", "fStaminaLossFlatAddition", staminaLossFlatAddition);

        ini.SetValue("Combat", nullptr, nullptr);
        ini.SetValue("Combat", nullptr, "; Enable stamina loss when blocking (regular block, not timed)");
        ini.SetBoolValue("Combat", "bEnableRegularBlockStaminaLossOnHit", enableRegularBlockStaminaLossOnHit);

        ini.SetValue("Combat", nullptr, nullptr);
        ini.SetValue("Combat", nullptr, "; --- Regular Block (Late/Missed Window) ---");
        ini.SetValue("Combat", nullptr, "; Stamina multiplier for regular blocks (0.5 = half loss, 2.0 = double loss)");
        ini.SetDoubleValue("Combat", "fRegularBlockStaminaMult", regularBlockStaminaMult);

        // ===== TIMED BLOCKING SYSTEM =====
        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; ============================================");
        ini.SetValue("TimedBlocking", nullptr, "; TIMED BLOCKING SYSTEM");
        ini.SetValue("TimedBlocking", nullptr, "; ============================================");
        ini.SetValue("TimedBlocking", nullptr, nullptr);

        ini.SetValue("TimedBlocking", nullptr, "; Enable timed blocking system");
        ini.SetBoolValue("TimedBlocking", "bEnableTimedBlocking", enableTimedBlocking);

        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; Time window for successful timed block (seconds)");
        ini.SetValue("TimedBlocking", nullptr, "; If you block and get hit within this window, it's a TIMED block");
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockWindow", timedBlockWindow);

        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; Animation delay before timed window starts (seconds)");
        ini.SetValue("TimedBlocking", nullptr, "; Allows block animation to play before window activates");
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockAnimationDelay", timedBlockAnimationDelay);

        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; Block button universal key code");
        ini.SetValue("TimedBlocking", nullptr, "; Mouse: 256=left, 257=right, 258=middle");
        ini.SetValue("TimedBlocking", nullptr, "; Keyboard: Use scan codes (e.g., 42=LShift)");
        ini.SetValue("TimedBlocking", nullptr, "; Gamepad: 266+ offset");
        ini.SetLongValue("TimedBlocking", "iBlockButton", blockButton);

        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; --- Timed Block (Perfect Timing) ---");
        ini.SetValue("TimedBlocking", nullptr, "; Enable stamina LOSS on timed block");
        ini.SetBoolValue("TimedBlocking", "bTimedBlockStaminaLoss", timedBlockStaminaLoss);

        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; Enable stamina GAIN on timed block");
        ini.SetBoolValue("TimedBlocking", "bTimedBlockStaminaGain", timedBlockStaminaGain);

        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; Flat stamina gain on timed block");
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockStaminaAmountGain", timedBlockStaminaAmountGain);

        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; Stamina loss multiplier (applied to regular block loss)");
        ini.SetValue("TimedBlocking", nullptr, "; Example: regular block loses 10, mult 0.5 = timed block loses 5");
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockStaminaAmountLossMult", timedBlockStaminaAmountLossMult);

        ini.SetValue("TimedBlocking", nullptr, nullptr);
        ini.SetValue("TimedBlocking", nullptr, "; Damage reduction for timed blocks (1.0 = 100% negated, 0.5 = 50% reduction)");
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockDamageReduction", timedBlockDamageReduction);


        // ===== EXHAUSTION SYSTEM =====  // new
        ini.SetValue("Exhaustion", nullptr, nullptr);
        ini.SetValue("Exhaustion", nullptr, "; ============================================");
        ini.SetValue("Exhaustion", nullptr, "; EXHAUSTION DEBUFF SYSTEM");
        ini.SetValue("Exhaustion", nullptr, "; ============================================");
        ini.SetValue("Exhaustion", nullptr, nullptr);

        ini.SetValue("Exhaustion", nullptr, "; Enable exhaustion debuffs when stamina is low");
        ini.SetBoolValue("Exhaustion", "bEnableExhaustionDebuff", enableExhaustionDebuff);

        ini.SetValue("Exhaustion", nullptr, nullptr);
        ini.SetValue("Exhaustion", nullptr, "; Stamina threshold - debuffs apply when current stamina < this value");
        ini.SetDoubleValue("Exhaustion", "fExhaustionStaminaThreshold", exhaustionStaminaThreshold);

        ini.SetValue("Exhaustion", nullptr, nullptr);
        ini.SetValue("Exhaustion", nullptr, "; Movement speed debuff (0.20 = 20% slower)");
        ini.SetDoubleValue("Exhaustion", "fExhaustionMovementSpeedDebuff", exhaustionMovementSpeedDebuff);

        ini.SetValue("Exhaustion", nullptr, "; Attack damage debuff (0.25 = 25% less damage dealt)");
        ini.SetDoubleValue("Exhaustion", "fExhaustionAttackDamageDebuff", exhaustionAttackDamageDebuff);

        ini.SetValue("Exhaustion", nullptr, "; Damage received multiplier (1.25 = 25% more damage taken)");
        ini.SetDoubleValue("Exhaustion", "fExhaustionDamageReceivedMult", exhaustionDamageReceivedMult);

        // ===== CASTING DEBUFF =====
        ini.SetValue("CastingDebuff", nullptr, nullptr);
        ini.SetValue("CastingDebuff", nullptr, "; ============================================");
        ini.SetValue("CastingDebuff", nullptr, "; CASTING DEBUFF SYSTEM");
        ini.SetValue("CastingDebuff", nullptr, "; ============================================");
        ini.SetValue("CastingDebuff", nullptr, nullptr);

        ini.SetValue("CastingDebuff", nullptr, "; Apply slowdown to NPCs in combat");
        ini.SetBoolValue("CastingDebuff", "bApplyToNPCs", applyToNPCs);

        // Bow
        ini.SetValue("CastingDebuff.Bow", nullptr, nullptr);
        ini.SetValue("CastingDebuff.Bow", nullptr, "; --- Bow Slowdown ---");
        ini.SetValue("CastingDebuff.Bow", nullptr, "; Skill ranges: Novice (0-25), Apprentice (26-50), Expert (51-75), Master (76-100)");
        ini.SetBoolValue("CastingDebuff.Bow", "bEnableBowDebuff", enableBowDebuff);
        ini.SetDoubleValue("CastingDebuff.Bow", "fNoviceMultiplier", bowMultipliers[0]);
        ini.SetDoubleValue("CastingDebuff.Bow", "fApprenticeMultiplier", bowMultipliers[1]);
        ini.SetDoubleValue("CastingDebuff.Bow", "fExpertMultiplier", bowMultipliers[2]);
        ini.SetDoubleValue("CastingDebuff.Bow", "fMasterMultiplier", bowMultipliers[3]);

        // Crossbow
        ini.SetValue("CastingDebuff.Crossbow", nullptr, nullptr);
        ini.SetValue("CastingDebuff.Crossbow", nullptr, "; --- Crossbow Slowdown ---");
        ini.SetValue("CastingDebuff.Crossbow", nullptr, "; Skill ranges: Novice (0-25), Apprentice (26-50), Expert (51-75), Master (76-100)");
        ini.SetBoolValue("CastingDebuff.Crossbow", "bEnableCrossbowDebuff", enableCrossbowDebuff);
        ini.SetDoubleValue("CastingDebuff.Crossbow", "fNoviceMultiplier", crossbowMultipliers[0]);
        ini.SetDoubleValue("CastingDebuff.Crossbow", "fApprenticeMultiplier", crossbowMultipliers[1]);
        ini.SetDoubleValue("CastingDebuff.Crossbow", "fExpertMultiplier", crossbowMultipliers[2]);
        ini.SetDoubleValue("CastingDebuff.Crossbow", "fMasterMultiplier", crossbowMultipliers[3]);

        // Cast
        ini.SetValue("CastingDebuff.Cast", nullptr, nullptr);
        ini.SetValue("CastingDebuff.Cast", nullptr, "; --- Magic Casting Slowdown ---");
        ini.SetValue("CastingDebuff.Cast", nullptr, "; Skill ranges based on spell school level");
        ini.SetBoolValue("CastingDebuff.Cast", "bEnableCastDebuff", enableCastDebuff);
        ini.SetDoubleValue("CastingDebuff.Cast", "fNoviceMultiplier", castMultipliers[0]);
        ini.SetDoubleValue("CastingDebuff.Cast", "fApprenticeMultiplier", castMultipliers[1]);
        ini.SetDoubleValue("CastingDebuff.Cast", "fExpertMultiplier", castMultipliers[2]);
        ini.SetDoubleValue("CastingDebuff.Cast", "fMasterMultiplier", castMultipliers[3]);

        // Dual Cast
        ini.SetValue("CastingDebuff.DualCast", nullptr, nullptr);
        ini.SetValue("CastingDebuff.DualCast", nullptr, "; --- Dual Casting Slowdown ---");
        ini.SetValue("CastingDebuff.DualCast", nullptr, "; Applied when casting with both hands simultaneously");
        ini.SetBoolValue("CastingDebuff.DualCast", "bEnableDualCastDebuff", enableDualCastDebuff);
        ini.SetDoubleValue("CastingDebuff.DualCast", "fNoviceMultiplier", dualCastMultipliers[0]);
        ini.SetDoubleValue("CastingDebuff.DualCast", "fApprenticeMultiplier", dualCastMultipliers[1]);
        ini.SetDoubleValue("CastingDebuff.DualCast", "fExpertMultiplier", dualCastMultipliers[2]);
        ini.SetDoubleValue("CastingDebuff.DualCast", "fMasterMultiplier", dualCastMultipliers[3]);

        // ===== DEBUG =====
        ini.SetValue("Debug", nullptr, nullptr);
        ini.SetValue("Debug", nullptr, "; ============================================");
        ini.SetValue("Debug", nullptr, "; DEBUG SETTINGS");
        ini.SetValue("Debug", nullptr, "; ============================================");
        ini.SetValue("Debug", nullptr, nullptr);

        ini.SetValue("Debug", nullptr, "; Log Level: 0=trace, 1=debug, 2=info, 3=warn, 4=error, 5=critical");
        ini.SetLongValue("Debug", "iLogLevel", logLevel);

        auto path = GetConfigPath();
        std::filesystem::create_directories(path.parent_path());
        ini.SaveFile(path.string().c_str());
        logger::info("Config saved to {}", path.string());
    }
}