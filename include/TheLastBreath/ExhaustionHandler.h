#pragma once

namespace TheLastBreath {

    class ExhaustionHandler {
    public:
        static ExhaustionHandler* GetSingleton() {
            static ExhaustionHandler singleton;
            return &singleton;
        }

        void Update();
        void ClearAll();

    private:
        ExhaustionHandler() = default;
        ExhaustionHandler(const ExhaustionHandler&) = delete;
        ExhaustionHandler(ExhaustionHandler&&) = delete;

        struct ExhaustionState {
            bool isExhausted = false;
            //bool jumpDisabled = false;
            //float originalJumpHeight = 76.0f;
        };

        std::unordered_map<RE::FormID, ExhaustionState> actorStates;

        void ApplyExhaustion(RE::Actor* actor);
        void RemoveExhaustion(RE::Actor* actor);
    };

}