#pragma once
#include <chrono>

namespace TheLastBreath {

    enum class BlockType {
        None,
        Timed,
        Regular
    };

    class TimedBlockHandler {
    public:
        static TimedBlockHandler* GetSingleton() {
            static TimedBlockHandler singleton;
            return &singleton;
        }

        void OnButtonPressed(RE::Actor* actor);
        void OnButtonReleased(RE::Actor* actor);
        BlockType CheckBlockType(RE::Actor* actor);
        void ConsumeTimedBlock(RE::Actor* actor);
        void Update();  // Check button state each frame
        void ClearActor(RE::Actor* actor);

    private:
        TimedBlockHandler() = default;
        TimedBlockHandler(const TimedBlockHandler&) = delete;
        TimedBlockHandler(TimedBlockHandler&&) = delete;

        struct BlockState {
            bool isButtonPressed = false;
            bool windowActive = false;
            bool windowConsumed = false;
            std::chrono::steady_clock::time_point buttonPressTime;
            std::chrono::steady_clock::time_point windowStartTime;
        };

        std::unordered_map<RE::FormID, BlockState> actorStates;
    };

}