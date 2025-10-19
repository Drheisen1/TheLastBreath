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
        bool enableBlockStaminaDrain = true;                    // new
        float blockHoldStaminaCostPerSecond = 2.0f;            // new

        // Stamina Loss on Hit
        bool enableStaminaLossOnHit = true;
        float staminaLossBaseIntercept = 14.5f;      // Base at 0 max stamina
        float staminaLossScalingFactor = 0.018f;     // How much to subtract per max stamina point
        float staminaLossFlatAddition = 1.0f;        // Flat amount always added
        bool enableRegularBlockStaminaLossOnHit = true;
        float regularBlockStaminaMult = 0.5f;

        // Melee Weapons
        bool enableLightAttackStamina = true;  // new
        float lightAttackStaminaCostMult = 0.3f; 

        // Ranged Weapons (Bow/Crossbow combined)
        bool enableRangedStaminaCost = true;
        bool enableRangedHoldStaminaDrain = true;     // new
        bool enableRangedReleaseStaminaCost = true;   // new
        float rangedHoldStaminaCostPerSecond = 3.0f;
        float rangedReleaseStaminaCost = 10.0f;
        bool enableRapidComboStaminaCost = false;      // new
		float rapidComboStaminaCost = 10.0f;           // new

        // Exhaustion System
        bool enableExhaustionDebuff = true;  // new
        float exhaustionStaminaThreshold = 20.0f; // new
        float exhaustionMovementSpeedDebuff = 0.20f;   // new    // 20% slower
        float exhaustionAttackDamageDebuff = 0.25f;   // new     // 25% less damage
        float exhaustionDamageReceivedMult = 1.25f;   // new     // 25% more damage taken

        // Timed Blocking System
        bool enableTimedBlocking = true;
        float timedBlockWindow = 0.2f;
        float timedBlockAnimationDelay = 0.25f;
        uint32_t blockButton = 257;
        bool timedBlockStaminaLoss = false;              // NEW - Enable stamina loss on timed block
        bool timedBlockStaminaGain = true;               // NEW - Enable stamina gain on timed block
        float timedBlockStaminaAmountGain = 20.0f;           // NEW - Flat stamina gain
        float timedBlockStaminaAmountLossMult = 0.5f;        // NEW - Multiplier of regular block loss
        float timedBlockDamageReduction = 1.0f;

        // ===== CASTING DEBUFF =====
        bool applyToNPCs = true;

        // Bow
        bool enableBowDebuff = true;
        std::array<float, 4> bowMultipliers = { 0.5f, 0.6f, 0.7f, 0.8f };

        // Crossbow
        bool enableCrossbowDebuff = true;
        std::array<float, 4> crossbowMultipliers = { 0.5f, 0.6f, 0.7f, 0.8f };

        // Cast
        bool enableCastDebuff = true;
        std::array<float, 4> castMultipliers = { 0.5f, 0.6f, 0.7f, 0.8f };

        // Dual Cast
        bool enableDualCastDebuff = true;
        std::array<float, 4> dualCastMultipliers = { 0.4f, 0.5f, 0.6f, 0.7f };

        // ===== DEBUG =====
        int logLevel = 1;  // 0=trace, 1=debug, 2=info, 3=warn, 4=error, 5=critical

        // Block Sound Effects (loaded from plugin)
        RE::FormID parryWeaponSound1 = 0;
        RE::FormID parryWeaponSound2 = 0;
        RE::FormID parryWeaponSound3 = 0;
        RE::FormID parryWeaponSound4 = 0;
        RE::FormID parryShieldSound1 = 0;
        RE::FormID parryShieldSound2 = 0;
        RE::FormID parryShieldSound3 = 0;
        RE::FormID parryShieldSound4 = 0;

    private:
        Config() = default;
        Config(const Config&) = delete;
        Config(Config&&) = delete;

        static std::filesystem::path GetConfigPath();
    };

}