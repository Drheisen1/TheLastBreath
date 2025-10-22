#include "TheLastBreath/ActorStateManager.h"

namespace TheLastBreath {

    ActorCombatState* ActorStateManager::GetState(RE::Actor* actor) {
        if (!actor) return nullptr;

        std::lock_guard<std::mutex> lock(stateMutex);
        auto [it, inserted] = actorStates.try_emplace(actor->GetFormID());
        return &it->second;
    }

    ActorCombatState* ActorStateManager::GetStateIfExists(RE::Actor* actor) {
        if (!actor) return nullptr;

        std::lock_guard<std::mutex> lock(stateMutex);
        auto it = actorStates.find(actor->GetFormID());
        return (it != actorStates.end()) ? &it->second : nullptr;
    }

    void ActorStateManager::ClearActor(RE::Actor* actor) {
        if (!actor) return;

        std::lock_guard<std::mutex> lock(stateMutex);
        actorStates.erase(actor->GetFormID());
    }

    void ActorStateManager::ClearAll() {
        std::lock_guard<std::mutex> lock(stateMutex);
        actorStates.clear();
        logger::info("Cleared all actor combat states");
    }
}
