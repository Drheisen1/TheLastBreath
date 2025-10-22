#pragma once
#include "TheLastBreath/Offsets.h"
#include <thread>
#include <chrono>

namespace TheLastBreath {

    namespace SlowTimeUtils {

        /**
         * Slow down game time for a set period.
         * @param duration Duration of the slow time in seconds.
         * @param percentage Relative time speed (0.1 = 10% speed, 1.0 = normal).
         * @param useSmoothing If true, smoothly transitions to target speed (cinematic).
         */
        inline void ApplySlowTime(float duration, float percentage, bool useSmoothing = true) {
            if (duration <= 0.0f || percentage <= 0.0f || percentage >= 1.0f) {
                return;
            }

            int durationMilliseconds = static_cast<int>(duration * 1000);

            // Apply slow time with smooth transition
            Offsets::SGTM(percentage, useSmoothing);
            logger::debug("Applied slow time: {:.0f}% speed for {:.1f}s (smooth: {})",
                percentage * 100.0f, duration, useSmoothing ? "yes" : "no");

            // Reset time after duration
            auto resetSlowTime = [useSmoothing](int durationMs) {
                std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
                Offsets::SGTM(1.0f, useSmoothing);
                logger::debug("Reset time to normal speed (smooth: {})", useSmoothing ? "yes" : "no");
                };

            std::jthread resetThread(resetSlowTime, durationMilliseconds);
            resetThread.detach();
        }

    }  // namespace SlowTimeUtils

}  // namespace TheLastBreath