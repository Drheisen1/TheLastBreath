#include "SIGA/CompatibilityMonitor.h"
#include "SIGA/SlowMotion.h"

// Not being used, keepoing for backup

namespace SIGA {
    void CompatibilityMonitor::Update() {
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        auto slowMgr = SlowMotionManager::GetSingleton();
        if (!slowMgr->IsActorSlowed(player)) return;

        // If slowed but not actually casting/drawing, force cleanup
        bool animCastingLeft = false;
        bool animCastingRight = false;
        bool animDrawingBow = false;

        // Call directly on the player actor
        player->GetGraphVariableBool("IsCastingLeft", animCastingLeft);
        player->GetGraphVariableBool("IsCastingRight", animCastingRight);
        player->GetGraphVariableBool("IsAttacking", animDrawingBow);

        if (!animCastingLeft && !animCastingRight && !animDrawingBow) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastActivityTime).count();

            // If no activity for elapsed time, force cleanup
            if (elapsed > 500) {
                logger::warn("Detected stuck slowdown, forcing cleanup");
                slowMgr->ClearAllSlowdowns(player);
            }
        }
        else {
            lastActivityTime = std::chrono::steady_clock::now();
        }
    }
}