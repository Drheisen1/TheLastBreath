#pragma once
#include <array>
#include <filesystem>

namespace TheLastBreath {
    class Config {
    public:
        static Config* GetSingleton() {
            static Config singleton;
            return &singleton;
        }

        void Load();
        void Save();

        // ===== STAMINA =====
        bool enableStaminaManagement = true;
        bool enableJumpStaminaCost = true;
        float jumpStaminaCost = 10.0f;
        bool enableBlockStaminaDrain = true;
        float blockHoldStaminaCostPerSecond = 2.0f;

        // Stamina Loss on Hit
        bool enableStaminaLossOnHit = true;
        float staminaLossBaseIntercept = 14.5f;
        float staminaLossScalingFactor = 0.018f;
        float staminaLossFlatAddition = 1.0f;
        bool enableRegularBlockStaminaLossOnHit = true;
        float regularBlockStaminaMult = 0.5f;

        // Melee Weapons
        bool enableLightAttackStamina = true;
        float lightAttackStaminaCostMult = 0.15f;

        // Ranged Weapons (Bow/Crossbow combined)
        bool enableRangedStaminaCost = true;
        bool enableRangedHoldStaminaDrain = true;
        bool enableRangedReleaseStaminaCost = true;
        float rangedHoldStaminaCostPerSecond = 3.0f;
        float rangedReleaseStaminaCost = 10.0f;
        bool enableRapidComboStaminaCost = false;
        float rapidComboStaminaCost = 10.0f;

        // Exhaustion System
        bool enableExhaustionDebuff = true;
        float exhaustionStaminaThreshold = 20.0f;
        float exhaustionMovementSpeedDebuff = 0.20f;      // 20% movement speed reduction
        float exhaustionAttackDamageDebuff = 0.25f;       // 25% attack damage reduction

        // Timed Blocking System
        bool enableTimedBlocking = true;
        bool enableTimedBlockSkillRequirement = false;
        float timedBlockRequiredSkillLevel = 0.0f;
        float timedBlockWindow1 = 0.3f;   // Parry 1 (easiest)
        float timedBlockWindow2 = 0.25f;  // Parry 2
        float timedBlockWindow3 = 0.2f;   // Parry 3
        float timedBlockWindow4 = 0.15f;  // Parry 4
        float timedBlockWindow5 = 0.1f;   // Parry 5 (hardest - perfect parry)
        float timedBlockAnimationDelay = 0.05f;
        uint32_t blockButton = 257;
        bool timedBlockStaminaLoss = false;
        bool timedBlockStaminaGain = true;
        float timedBlockStaminaAmountGain = 20.0f;
        float timedBlockStaminaAmountLossMult = 0.5f;
        float timedBlockDamageReduction = 1.0f;  // 1.0 = 100% damage reduction
        bool slowTimeOnlyOnPerfectParry = true;   // Only on parry 5, or all parries?
        float slowTimeDuration = 0.5f;            // Duration in seconds
        float slowTimePercentage = 0.4f;          // Time speed (0.1 = 10% speed)


        // Parry Sequence System
        bool enableParryStagger = true;
        bool enablePerfectParry = true;
        float parrySequenceTimeoutBase = 2.0f;           // Base timeout (5 seconds)
        float parryStaggerMagnitude1 = 0.1f;             // Parry 1 stagger
        float parryStaggerMagnitude2 = 0.2f;             // Parry 2 stagger
        float parryStaggerMagnitude3 = 0.3f;             // Parry 3 stagger
        float parryStaggerMagnitude4 = 0.4f;             // Parry 4 stagger
        float perfectParryStaggerMagnitude = 10.0f;      // Perfect parry (5)
        float parrySoundVolume = 1.0f;  // 0.0 = mute, 1.0 = full volume
        bool enableParrySparks = true;  // Toggle visual sparks


        // Elden Counter Integration
        bool enableEldenCounter = false;                    // Master toggle
        bool eldenCounterOnlyTimedBlocks = true;            // Only on successful timed blocks
        bool eldenCounterOnlyPerfectParry = false;          // Only on perfect parry (5th)

        // ===== NPCS =====
        bool applyToNPCs = true;

        // ===== DEBUG =====
        int logLevel = 1;  // 0=trace, 1=debug, 2=info, 3=warn, 4=error, 5=critical

        // ===== BLOCK VISUAL EFFECTS (loaded from plugin) =====
        // Base activator for spawning FX
        RE::FormID blockSparkTempMark = 0;

        // Addon nodes with embedded sounds and visuals
        RE::FormID parryWeaponAddon1 = 0;
        RE::FormID parryWeaponAddon2 = 0;
        RE::FormID parryWeaponAddon3 = 0;
        RE::FormID parryWeaponAddon4 = 0;
        RE::FormID parryShieldAddon1 = 0;
        RE::FormID parryShieldAddon2 = 0;
        RE::FormID parryShieldAddon3 = 0;
        RE::FormID parryShieldAddon4 = 0;

        //// Sound descriptors (kept as fallback, but addon nodes handle sounds)
        //RE::FormID parryWeaponSound1 = 0;
        //RE::FormID parryWeaponSound2 = 0;
        //RE::FormID parryWeaponSound3 = 0;
        //RE::FormID parryWeaponSound4 = 0;
        //RE::FormID parryShieldSound1 = 0;
        //RE::FormID parryShieldSound2 = 0;
        //RE::FormID parryShieldSound3 = 0;
        //RE::FormID parryShieldSound4 = 0;

    private:
        Config() = default;
        Config(const Config&) = delete;
        Config(Config&&) = delete;

        static std::filesystem::path GetConfigPath();
    };
}