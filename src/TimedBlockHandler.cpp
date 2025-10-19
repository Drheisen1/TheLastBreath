#include "TheLastBreath/TimedBlockHandler.h"
#include "TheLastBreath/Config.h"

namespace TheLastBreath {

    void TimedBlockHandler::OnButtonPressed(RE::Actor* actor) {
        if (!actor) return;

        auto config = Config::GetSingleton();
        if (!config->enableTimedBlocking) return;

        auto formID = actor->GetFormID();
        auto& state = actorStates[formID];

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
                    logger::debug("Timed block window NOW active ({:.1f}s)", config->timedBlockWindow);
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

        // Check if window is active (animation delay passed)
        if (!state.windowActive) {
            logger::debug("Block window not yet active (in animation delay) - regular block");
            return BlockType::Regular;
        }

        // Check if hit is within timed window
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - state.windowStartTime).count();

        float elapsedSeconds = static_cast<float>(elapsed) / 1000.0f;

        if (elapsedSeconds <= config->timedBlockWindow) {
            logger::debug("TIMED BLOCK! Hit within window ({:.3f}s / {:.1f}s)",
                elapsedSeconds, config->timedBlockWindow);
            return BlockType::Timed;
        }
        else {
            logger::debug("Regular block - hit outside window ({:.3f}s / {:.1f}s)",
                elapsedSeconds, config->timedBlockWindow);
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