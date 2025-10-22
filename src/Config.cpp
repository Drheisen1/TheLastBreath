#include "TheLastBreath/Config.h"
#include <SimpleIni.h>

namespace TheLastBreath {

    std::filesystem::path Config::GetConfigPath() {
        return std::filesystem::path("Data/SKSE/Plugins/TheLastBreath.ini");
    }

    void Config::Load() {
        CSimpleIniA ini;
        ini.SetUnicode();

        auto path = GetConfigPath();
        SI_Error rc = ini.LoadFile(path.string().c_str());

        if (rc < 0) {
            logger::warn("Failed to load config file, using defaults");
            Save();
            return;
        }

        // [Stamina]
        enableStaminaManagement = ini.GetBoolValue("Stamina", "bEnableStaminaManagement", true);
        enableJumpStaminaCost = ini.GetBoolValue("Stamina", "bEnableJumpStaminaCost", true);
        jumpStaminaCost = static_cast<float>(ini.GetDoubleValue("Stamina", "fJumpStaminaCost", 10.0));
        enableBlockStaminaDrain = ini.GetBoolValue("Stamina", "bEnableBlockStaminaDrain", true);
        blockHoldStaminaCostPerSecond = static_cast<float>(ini.GetDoubleValue("Stamina", "fBlockHoldStaminaCostPerSecond", 2.0));

        enableLightAttackStamina = ini.GetBoolValue("Stamina", "bEnableLightAttackStamina", false);
        lightAttackStaminaCostMult = static_cast<float>(ini.GetDoubleValue("Stamina", "fLightAttackStaminaCost", 0.15));

        enableRangedStaminaCost = ini.GetBoolValue("Stamina", "bEnableRangedStaminaCost", true);
        enableRangedHoldStaminaDrain = ini.GetBoolValue("Stamina", "bEnableRangedHoldStaminaDrain", false);
        rangedHoldStaminaCostPerSecond = static_cast<float>(ini.GetDoubleValue("Stamina", "fRangedHoldStaminaCostPerSecond", 3.0));
        enableRangedReleaseStaminaCost = ini.GetBoolValue("Stamina", "bEnableRangedReleaseStaminaCost", true);
        rangedReleaseStaminaCost = static_cast<float>(ini.GetDoubleValue("Stamina", "fRangedReleaseStaminaCost", 10.0));
        enableRapidComboStaminaCost = ini.GetBoolValue("Stamina", "bEnableRapidComboStaminaCost", true);
        rapidComboStaminaCost = static_cast<float>(ini.GetDoubleValue("Stamina", "fRapidComboStaminaCost", 10.0));

        // [Combat]
        enableStaminaLossOnHit = ini.GetBoolValue("Combat", "bEnableStaminaLossOnHit", true);
        staminaLossBaseIntercept = static_cast<float>(ini.GetDoubleValue("Combat", "fStaminaLossBaseIntercept", 14.5));
        staminaLossScalingFactor = static_cast<float>(ini.GetDoubleValue("Combat", "fStaminaLossScalingFactor", 0.018));
        staminaLossFlatAddition = static_cast<float>(ini.GetDoubleValue("Combat", "fStaminaLossFlatAddition", 1.0));
        enableRegularBlockStaminaLossOnHit = ini.GetBoolValue("Combat", "bEnableRegularBlockStaminaLossOnHit", true);
        regularBlockStaminaMult = static_cast<float>(ini.GetDoubleValue("Combat", "fRegularBlockStaminaMult", 0.5));

        // [Exhaustion]
        enableExhaustionDebuff = ini.GetBoolValue("Exhaustion", "bEnableExhaustionDebuff", true);
        exhaustionStaminaThreshold = static_cast<float>(ini.GetDoubleValue("Exhaustion", "fExhaustionStaminaThreshold", 20.0));
        exhaustionMovementSpeedDebuff = static_cast<float>(ini.GetDoubleValue("Exhaustion", "fExhaustionMovementSpeedDebuff", 0.20));
        exhaustionAttackDamageDebuff = static_cast<float>(ini.GetDoubleValue("Exhaustion", "fExhaustionAttackDamageDebuff", 0.25));
        exhaustionDamageReceivedMult = static_cast<float>(ini.GetDoubleValue("Exhaustion", "fExhaustionDamageReceivedMult", 1.25));

        // [TimedBlocking]
        enableTimedBlocking = ini.GetBoolValue("TimedBlocking", "bEnableTimedBlocking", true);
        timedBlockWindow1 = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockWindow1", 0.3));
        timedBlockWindow2 = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockWindow2", 0.25));
        timedBlockWindow3 = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockWindow3", 0.2));
        timedBlockWindow4 = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockWindow4", 0.15));
        timedBlockWindow5 = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockWindow5", 0.1));
        timedBlockAnimationDelay = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockAnimationDelay", 0.05));
        blockButton = static_cast<uint32_t>(ini.GetLongValue("TimedBlocking", "iBlockButton", 257));
        timedBlockStaminaLoss = ini.GetBoolValue("TimedBlocking", "bTimedBlockStaminaLoss", false);
        timedBlockStaminaGain = ini.GetBoolValue("TimedBlocking", "bTimedBlockStaminaGain", true);
        timedBlockStaminaAmountGain = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockStaminaAmountGain", 20.0));
        timedBlockStaminaAmountLossMult = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockStaminaAmountLossMult", 0.5));
        timedBlockDamageReduction = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fTimedBlockDamageReduction", 1.0));
        slowTimeOnlyOnPerfectParry = ini.GetBoolValue("TimedBlocking", "bSlowTimeOnlyOnPerfectParry", true);
        slowTimeDuration = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fSlowTimeDuration", 0.5));
        slowTimePercentage = static_cast<float>(ini.GetDoubleValue("TimedBlocking", "fSlowTimePercentage", 0.4));

        // [ParrySystem]
        enableParryStagger = ini.GetBoolValue("ParrySystem", "bEnableParryStagger", true);
        enablePerfectParry = ini.GetBoolValue("ParrySystem", "bEnablePerfectParry", true);
        parrySequenceTimeoutBase = static_cast<float>(ini.GetDoubleValue("ParrySystem", "fParrySequenceTimeoutBase", 2.0));
        parryStaggerMagnitude1 = static_cast<float>(ini.GetDoubleValue("ParrySystem", "fParryStaggerMagnitude1", 0.1));
        parryStaggerMagnitude2 = static_cast<float>(ini.GetDoubleValue("ParrySystem", "fParryStaggerMagnitude2", 0.2));
        parryStaggerMagnitude3 = static_cast<float>(ini.GetDoubleValue("ParrySystem", "fParryStaggerMagnitude3", 0.3));
        parryStaggerMagnitude4 = static_cast<float>(ini.GetDoubleValue("ParrySystem", "fParryStaggerMagnitude4", 0.4));
        perfectParryStaggerMagnitude = static_cast<float>(ini.GetDoubleValue("ParrySystem", "fPerfectParryStaggerMagnitude", 10.0));
        parrySoundVolume = static_cast<float>(ini.GetDoubleValue("ParrySystem", "fParrySoundVolume", 1.0));
        enableParrySparks = ini.GetBoolValue("ParrySystem", "bEnableParrySparks", true);

        // [EldenCounter]
        enableEldenCounter = ini.GetBoolValue("EldenCounter", "bEnableEldenCounter", false);
        eldenCounterOnlyTimedBlocks = ini.GetBoolValue("EldenCounter", "bEldenCounterOnlyTimedBlocks", true);
        eldenCounterOnlyPerfectParry = ini.GetBoolValue("EldenCounter", "bEldenCounterOnlyPerfectParry", false);
        eldenCounterEnableIFrames = ini.GetBoolValue("EldenCounter", "bEldenCounterEnableIFrames", true);
        eldenCounterDuration = static_cast<float>(ini.GetDoubleValue("EldenCounter", "fEldenCounterDuration", 1.5));
        eldenCounterIFrameDuration = static_cast<float>(ini.GetDoubleValue("EldenCounter", "fEldenCounterIFrameDuration", 1.0));

        // [NPCs]
        applyToNPCs = ini.GetBoolValue("NPCs", "bApplyToNPCs", true);

        // [Bow]
        enableBowDebuff = ini.GetBoolValue("Bow", "bEnableBowDebuff", true);
        bowMultipliers[0] = static_cast<float>(ini.GetDoubleValue("Bow", "fBowNoviceMultiplier", 0.5));
        bowMultipliers[1] = static_cast<float>(ini.GetDoubleValue("Bow", "fBowApprenticeMultiplier", 0.6));
        bowMultipliers[2] = static_cast<float>(ini.GetDoubleValue("Bow", "fBowExpertMultiplier", 0.7));
        bowMultipliers[3] = static_cast<float>(ini.GetDoubleValue("Bow", "fBowMasterMultiplier", 0.8));

        // [Crossbow]
        enableCrossbowDebuff = ini.GetBoolValue("Crossbow", "bEnableCrossbowDebuff", true);
        crossbowMultipliers[0] = static_cast<float>(ini.GetDoubleValue("Crossbow", "fCrossbowNoviceMultiplier", 0.5));
        crossbowMultipliers[1] = static_cast<float>(ini.GetDoubleValue("Crossbow", "fCrossbowApprenticeMultiplier", 0.6));
        crossbowMultipliers[2] = static_cast<float>(ini.GetDoubleValue("Crossbow", "fCrossbowExpertMultiplier", 0.7));
        crossbowMultipliers[3] = static_cast<float>(ini.GetDoubleValue("Crossbow", "fCrossbowMasterMultiplier", 0.8));

        // [Cast]
        enableCastDebuff = ini.GetBoolValue("Cast", "bEnableCastDebuff", true);
        castMultipliers[0] = static_cast<float>(ini.GetDoubleValue("Cast", "fCastNoviceMultiplier", 0.5));
        castMultipliers[1] = static_cast<float>(ini.GetDoubleValue("Cast", "fCastApprenticeMultiplier", 0.6));
        castMultipliers[2] = static_cast<float>(ini.GetDoubleValue("Cast", "fCastExpertMultiplier", 0.7));
        castMultipliers[3] = static_cast<float>(ini.GetDoubleValue("Cast", "fCastMasterMultiplier", 0.8));

        // [DualCast]
        enableDualCastDebuff = ini.GetBoolValue("DualCast", "bEnableDualCastDebuff", true);
        dualCastMultipliers[0] = static_cast<float>(ini.GetDoubleValue("DualCast", "fDualCastNoviceMultiplier", 0.4));
        dualCastMultipliers[1] = static_cast<float>(ini.GetDoubleValue("DualCast", "fDualCastApprenticeMultiplier", 0.5));
        dualCastMultipliers[2] = static_cast<float>(ini.GetDoubleValue("DualCast", "fDualCastExpertMultiplier", 0.6));
        dualCastMultipliers[3] = static_cast<float>(ini.GetDoubleValue("DualCast", "fDualCastMasterMultiplier", 0.7));

        // [Debug]
        logLevel = static_cast<int>(ini.GetLongValue("Debug", "iLogLevel", 1));

        logger::info("Configuration loaded successfully");
    }

    void Config::Save() {
        CSimpleIniA ini;
        ini.SetUnicode();

        ini.SetBoolValue("Stamina", "bEnableStaminaManagement", enableStaminaManagement);
        ini.SetBoolValue("Stamina", "bEnableJumpStaminaCost", enableJumpStaminaCost);
        ini.SetDoubleValue("Stamina", "fJumpStaminaCost", jumpStaminaCost);
        ini.SetBoolValue("Stamina", "bEnableBlockStaminaDrain", enableBlockStaminaDrain);
        ini.SetDoubleValue("Stamina", "fBlockHoldStaminaCostPerSecond", blockHoldStaminaCostPerSecond);
        ini.SetBoolValue("Stamina", "bEnableLightAttackStamina", enableLightAttackStamina);
        ini.SetDoubleValue("Stamina", "fLightAttackStaminaCost", lightAttackStaminaCostMult);
        ini.SetBoolValue("Stamina", "bEnableRangedStaminaCost", enableRangedStaminaCost);
        ini.SetBoolValue("Stamina", "bEnableRangedHoldStaminaDrain", enableRangedHoldStaminaDrain);
        ini.SetDoubleValue("Stamina", "fRangedHoldStaminaCostPerSecond", rangedHoldStaminaCostPerSecond);
        ini.SetBoolValue("Stamina", "bEnableRangedReleaseStaminaCost", enableRangedReleaseStaminaCost);
        ini.SetDoubleValue("Stamina", "fRangedReleaseStaminaCost", rangedReleaseStaminaCost);
        ini.SetBoolValue("Stamina", "bEnableRapidComboStaminaCost", enableRapidComboStaminaCost);
        ini.SetDoubleValue("Stamina", "fRapidComboStaminaCost", rapidComboStaminaCost);

        ini.SetBoolValue("Combat", "bEnableStaminaLossOnHit", enableStaminaLossOnHit);
        ini.SetDoubleValue("Combat", "fStaminaLossBaseIntercept", staminaLossBaseIntercept);
        ini.SetDoubleValue("Combat", "fStaminaLossScalingFactor", staminaLossScalingFactor);
        ini.SetDoubleValue("Combat", "fStaminaLossFlatAddition", staminaLossFlatAddition);
        ini.SetBoolValue("Combat", "bEnableRegularBlockStaminaLossOnHit", enableRegularBlockStaminaLossOnHit);
        ini.SetDoubleValue("Combat", "fRegularBlockStaminaMult", regularBlockStaminaMult);

        ini.SetBoolValue("Exhaustion", "bEnableExhaustionDebuff", enableExhaustionDebuff);
        ini.SetDoubleValue("Exhaustion", "fExhaustionStaminaThreshold", exhaustionStaminaThreshold);
        ini.SetDoubleValue("Exhaustion", "fExhaustionMovementSpeedDebuff", exhaustionMovementSpeedDebuff);
        ini.SetDoubleValue("Exhaustion", "fExhaustionAttackDamageDebuff", exhaustionAttackDamageDebuff);
        ini.SetDoubleValue("Exhaustion", "fExhaustionDamageReceivedMult", exhaustionDamageReceivedMult);

        ini.SetBoolValue("TimedBlocking", "bEnableTimedBlocking", enableTimedBlocking);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockWindow1", timedBlockWindow1);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockWindow2", timedBlockWindow2);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockWindow3", timedBlockWindow3);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockWindow4", timedBlockWindow4);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockWindow5", timedBlockWindow5);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockAnimationDelay", timedBlockAnimationDelay);
        ini.SetLongValue("TimedBlocking", "iBlockButton", blockButton);
        ini.SetBoolValue("TimedBlocking", "bTimedBlockStaminaLoss", timedBlockStaminaLoss);
        ini.SetBoolValue("TimedBlocking", "bTimedBlockStaminaGain", timedBlockStaminaGain);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockStaminaAmountGain", timedBlockStaminaAmountGain);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockStaminaAmountLossMult", timedBlockStaminaAmountLossMult);
        ini.SetDoubleValue("TimedBlocking", "fTimedBlockDamageReduction", timedBlockDamageReduction);
        ini.SetBoolValue("TimedBlocking", "bSlowTimeOnlyOnPerfectParry", slowTimeOnlyOnPerfectParry);
        ini.SetDoubleValue("TimedBlocking", "fSlowTimeDuration", slowTimeDuration);
        ini.SetDoubleValue("TimedBlocking", "fSlowTimePercentage", slowTimePercentage);


        ini.SetBoolValue("ParrySystem", "bEnableParryStagger", enableParryStagger);
        ini.SetBoolValue("ParrySystem", "bEnablePerfectParry", enablePerfectParry);
        ini.SetDoubleValue("ParrySystem", "fParrySequenceTimeoutBase", parrySequenceTimeoutBase);
        ini.SetDoubleValue("ParrySystem", "fParryStaggerMagnitude1", parryStaggerMagnitude1);
        ini.SetDoubleValue("ParrySystem", "fParryStaggerMagnitude2", parryStaggerMagnitude2);
        ini.SetDoubleValue("ParrySystem", "fParryStaggerMagnitude3", parryStaggerMagnitude3);
        ini.SetDoubleValue("ParrySystem", "fParryStaggerMagnitude4", parryStaggerMagnitude4);
        ini.SetDoubleValue("ParrySystem", "fPerfectParryStaggerMagnitude", perfectParryStaggerMagnitude);
        ini.SetDoubleValue("ParrySystem", "fParrySoundVolume", parrySoundVolume);
        ini.SetBoolValue("ParrySystem", "bEnableParrySparks", enableParrySparks);

        ini.SetBoolValue("EldenCounter", "bEnableEldenCounter", enableEldenCounter);
        ini.SetBoolValue("EldenCounter", "bEldenCounterOnlyTimedBlocks", eldenCounterOnlyTimedBlocks);
        ini.SetBoolValue("EldenCounter", "bEldenCounterOnlyPerfectParry", eldenCounterOnlyPerfectParry);
        ini.SetBoolValue("EldenCounter", "bEnableIFrames", eldenCounterEnableIFrames);
        ini.SetDoubleValue("EldenCounter", "fEldenCounterDuration", eldenCounterDuration);
        ini.SetDoubleValue("EldenCounter", "fEldenCounterIFrameDuration", eldenCounterIFrameDuration);

        ini.SetBoolValue("NPCs", "bApplyToNPCs", applyToNPCs);

        ini.SetBoolValue("Bow", "bEnableBowDebuff", enableBowDebuff);
        ini.SetDoubleValue("Bow", "fBowNoviceMultiplier", bowMultipliers[0]);
        ini.SetDoubleValue("Bow", "fBowApprenticeMultiplier", bowMultipliers[1]);
        ini.SetDoubleValue("Bow", "fBowExpertMultiplier", bowMultipliers[2]);
        ini.SetDoubleValue("Bow", "fBowMasterMultiplier", bowMultipliers[3]);

        ini.SetBoolValue("Crossbow", "bEnableCrossbowDebuff", enableCrossbowDebuff);
        ini.SetDoubleValue("Crossbow", "fCrossbowNoviceMultiplier", crossbowMultipliers[0]);
        ini.SetDoubleValue("Crossbow", "fCrossbowApprenticeMultiplier", crossbowMultipliers[1]);
        ini.SetDoubleValue("Crossbow", "fCrossbowExpertMultiplier", crossbowMultipliers[2]);
        ini.SetDoubleValue("Crossbow", "fCrossbowMasterMultiplier", crossbowMultipliers[3]);

        ini.SetBoolValue("Cast", "bEnableCastDebuff", enableCastDebuff);
        ini.SetDoubleValue("Cast", "fCastNoviceMultiplier", castMultipliers[0]);
        ini.SetDoubleValue("Cast", "fCastApprenticeMultiplier", castMultipliers[1]);
        ini.SetDoubleValue("Cast", "fCastExpertMultiplier", castMultipliers[2]);
        ini.SetDoubleValue("Cast", "fCastMasterMultiplier", castMultipliers[3]);

        ini.SetBoolValue("DualCast", "bEnableDualCastDebuff", enableDualCastDebuff);
        ini.SetDoubleValue("DualCast", "fDualCastNoviceMultiplier", dualCastMultipliers[0]);
        ini.SetDoubleValue("DualCast", "fDualCastApprenticeMultiplier", dualCastMultipliers[1]);
        ini.SetDoubleValue("DualCast", "fDualCastExpertMultiplier", dualCastMultipliers[2]);
        ini.SetDoubleValue("DualCast", "fDualCastMasterMultiplier", dualCastMultipliers[3]);

        ini.SetLongValue("Debug", "iLogLevel", logLevel);

        auto path = GetConfigPath();
        ini.SaveFile(path.string().c_str());
    }

}