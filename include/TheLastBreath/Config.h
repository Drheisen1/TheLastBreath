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

    private:
        Config() = default;
        Config(const Config&) = delete;
        Config(Config&&) = delete;

        static std::filesystem::path GetConfigPath();
    };
}