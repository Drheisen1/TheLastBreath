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

        // Parry Sequence System
        ini.SetValue("ParrySequence", nullptr, nullptr);
        ini.SetValue("ParrySequence", nullptr, "; ============================================");
        ini.SetValue("ParrySequence", nullptr, "; PARRY SEQUENCE SYSTEM");
        ini.SetValue("ParrySequence", nullptr, "; Escalating stagger system - each successful parry increases stagger");
        ini.SetValue("ParrySequence", nullptr, "; ============================================");
        ini.SetValue("ParrySequence", nullptr, nullptr);

        ini.SetValue("ParrySequence", nullptr, "; Enable light stagger on parries 1-4");
        ini.SetBoolValue("ParrySequence", "bEnableParryStagger", enableParryStagger);

        ini.SetValue("ParrySequence", nullptr, nullptr);
        ini.SetValue("ParrySequence", nullptr, "; Enable perfect parry (parry 5) with guard break");
        ini.SetBoolValue("ParrySequence", "bEnablePerfectParry", enablePerfectParry);

        ini.SetValue("ParrySequence", nullptr, nullptr);
        ini.SetValue("ParrySequence", nullptr, "; Base timeout in seconds (increases by +1, +2, +3, +4 after each parry)");
        ini.SetDoubleValue("ParrySequence", "fParrySequenceTimeoutBase", parrySequenceTimeoutBase);

        ini.SetValue("ParrySequence", nullptr, nullptr);
        ini.SetValue("ParrySequence", nullptr, "; Stagger magnitude for each parry level");
        ini.SetValue("ParrySequence", nullptr, "; Valhalla reference: 0.0=tiny, 0.3=medium, 0.7=large, 10.0=knockdown");
        ini.SetDoubleValue("ParrySequence", "fParryStaggerMagnitude1", parryStaggerMagnitude1);
        ini.SetDoubleValue("ParrySequence", "fParryStaggerMagnitude2", parryStaggerMagnitude2);
        ini.SetDoubleValue("ParrySequence", "fParryStaggerMagnitude3", parryStaggerMagnitude3);
        ini.SetDoubleValue("ParrySequence", "fParryStaggerMagnitude4", parryStaggerMagnitude4);

        ini.SetValue("ParrySequence", nullptr, nullptr);
        ini.SetValue("ParrySequence", nullptr, "; Perfect parry (parry 5) guard break magnitude");
        ini.SetDoubleValue("ParrySequence", "fPerfectParryStaggerMagnitude", perfectParryStaggerMagnitude);

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