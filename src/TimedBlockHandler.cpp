#include "TheLastBreath/TimedBlockHandler.h"
#include "TheLastBreath/BlockEffectsHandler.h"  // ADDED - Need to get parry count
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void TimedBlockHandler::OnButtonPressed(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        if (!config->enableTimedBlocking) return;

        auto formID = actor->GetFormID();

        // OPTIMIZATION: Use try_emplace instead of operator[]
        auto [it, inserted] = actorStates.try_emplace(formID);
        auto& state = it->second;

        state.isButtonPressed = true;
        state.windowActive = false;  // Not active yet - waiting for animation delay
        state.windowConsumed = false;
        state.buttonPressTime = std::chrono::steady_clock::now();
        state.windowStartTime = state.buttonPressTime;  // Will be updated after delay

        logger::debug("Block button pressed - animation delay: {:.3f}s", config->timedBlockAnimationDelay);
    }

    void TimedBlockHandler::OnButtonReleased(RE::Actor* actor) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        if (auto it = actorStates.find(formID); it != actorStates.end()) {
            actorStates.erase(it);
            logger::debug("Block button released - state cleared");
        }
    }

    bool TimedBlockHandler::IsTimedBlockWindowActive(RE::Actor* actor) const {
        if (!actor) return false;

        auto it = actorStates.find(actor->GetFormID());
        if (it == actorStates.end()) {
            return false;
        }

        const auto& state = it->second;

        // Window must be active and not yet consumed
        return state.windowActive && !state.windowConsumed;
    }

    void TimedBlockHandler::Update() {
        auto config = Config::GetSingleton();
        if (!config->enableTimedBlocking) return;

        auto now = std::chrono::steady_clock::now();

        for (auto& [formID, state] : actorStates) {
            // Check if animation delay has passed and window should activate
            if (state.isButtonPressed && !state.windowActive && !state.windowConsumed) {
                auto timeSincePress = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - state.buttonPressTime).count() / 1000.0f;

                if (timeSincePress >= config->timedBlockAnimationDelay) {
                    state.windowActive = true;
                    state.windowStartTime = now;

                    // Get actor and determine which window will be used
                    RE::Actor* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
                    if (actor) {
                        auto blockEffects = BlockEffectsHandler::GetSingleton();
                        uint32_t currentParryCount = blockEffects->GetCurrentParryCount(actor);
                        uint32_t nextParryLevel = currentParryCount + 1;

                        float windowDuration = 0.0f;
                        switch (nextParryLevel) {
                        case 1: windowDuration = config->timedBlockWindow1; break;
                        case 2: windowDuration = config->timedBlockWindow2; break;
                        case 3: windowDuration = config->timedBlockWindow3; break;
                        case 4: windowDuration = config->timedBlockWindow4; break;
                        case 5: windowDuration = config->timedBlockWindow5; break;
                        default: windowDuration = config->timedBlockWindow3; break;
                        }

                        logger::debug("Timed block window NOW active - Parry {} window: {:.3f}s",
                            nextParryLevel, windowDuration);
                    }
                }
            }
        }
    }

    BlockType TimedBlockHandler::CheckBlockType(RE::Actor* actor) {
        if (!actor) return BlockType::None;

        auto config = Config::GetSingleton();
        if (!config->enableTimedBlocking) return BlockType::Regular;

        auto formID = actor->GetFormID();
        auto it = actorStates.find(formID);

        if (it == actorStates.end()) {
            return BlockType::None;  // Not blocking
        }

        auto& state = it->second;

        if (!state.isButtonPressed) {
            return BlockType::None;
        }

        // Check if window was already consumed
        if (state.windowConsumed) {
            logger::debug("Block window already consumed - regular block");
            return BlockType::Regular;
        }

        // ============================================
        // CHECK ANIMATION DELAY DIRECTLY (don't rely on Update() flag)
        // ============================================
        auto now = std::chrono::steady_clock::now();
        auto timeSincePress = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - state.buttonPressTime).count() / 1000.0f;

        // Animation delay not passed yet
        if (timeSincePress < config->timedBlockAnimationDelay) {
            logger::debug("Block window not yet active - in animation delay ({:.3f}s / {:.3f}s)",
                timeSincePress, config->timedBlockAnimationDelay);
            return BlockType::Regular;
        }

        // Animation delay passed - now check if within parry window
        // Calculate time since delay passed (for window duration check)
        float timeInWindow = timeSincePress - config->timedBlockAnimationDelay;

        // ============================================
        // PROGRESSIVE WINDOW SYSTEM
        // ============================================

        // Get current parry level from BlockEffectsHandler
        auto blockEffects = BlockEffectsHandler::GetSingleton();
        uint32_t currentParryCount = blockEffects->GetCurrentParryCount(actor);
        uint32_t nextParryLevel = currentParryCount + 1;  // Next parry will be 1-5

        // Select appropriate window based on next parry level
        float windowDuration = 0.0f;
        switch (nextParryLevel) {
        case 1: windowDuration = config->timedBlockWindow1; break;
        case 2: windowDuration = config->timedBlockWindow2; break;
        case 3: windowDuration = config->timedBlockWindow3; break;
        case 4: windowDuration = config->timedBlockWindow4; break;
        case 5: windowDuration = config->timedBlockWindow5; break;
        default: windowDuration = config->timedBlockWindow3; break;  // Fallback
        }

        // Check if hit is within the window
        if (timeInWindow <= windowDuration) {
            logger::info("TIMED BLOCK! Parry {} window ({:.3f}s / {:.3f}s)",
                nextParryLevel,
                timeInWindow,
                windowDuration);
            return BlockType::Timed;
        }
        else {
            logger::debug("Regular block - missed Parry {} window ({:.3f}s / {:.3f}s)",
                nextParryLevel,
                timeInWindow,
                windowDuration);
            return BlockType::Regular;
        }
    }

    void TimedBlockHandler::ConsumeTimedBlock(RE::Actor* actor) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        if (auto it = actorStates.find(formID); it != actorStates.end()) {
            it->second.windowConsumed = true;
            logger::debug("Timed block window consumed - next hit will be regular block");
        }
    }

    void TimedBlockHandler::ClearActor(RE::Actor* actor) {
        if (!actor) return;

        auto formID = actor->GetFormID();
        if (auto it = actorStates.find(formID); it != actorStates.end()) {
            actorStates.erase(it);
            logger::debug("Cleared timed block state for actor");
        }
    }

}
