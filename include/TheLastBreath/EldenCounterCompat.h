#pragma once

namespace TheLastBreath {

    class EldenCounterCompat {
    public:
        static EldenCounterCompat* GetSingleton() {
            static EldenCounterCompat singleton;
            return &singleton;
        }

        void Initialize();
        bool IsAvailable() const { return isAvailable; }

        // Just apply trigger spell - EC's DLL does the rest
        void TriggerCounter(RE::Actor* actor, bool isPerfectParry);

    private:
        EldenCounterCompat() = default;
        EldenCounterCompat(const EldenCounterCompat&) = delete;
        EldenCounterCompat(EldenCounterCompat&&) = delete;

        bool isAvailable = false;
        RE::SpellItem* triggerSpell = nullptr;  // 0x801 - EC's trigger

        void FindTriggerSpell(const RE::TESFile* ecFile);
    };

}