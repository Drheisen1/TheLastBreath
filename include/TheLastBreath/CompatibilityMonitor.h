#pragma once

#include <chrono>

namespace TheLastBreath {
    class CompatibilityMonitor {
    public:
        static CompatibilityMonitor* GetSingleton() {
            static CompatibilityMonitor singleton;
            return &singleton;
        }

        void Update();

    private:
        CompatibilityMonitor() = default;
        CompatibilityMonitor(const CompatibilityMonitor&) = delete;
        CompatibilityMonitor(CompatibilityMonitor&&) = delete;

        std::chrono::steady_clock::time_point lastActivityTime = std::chrono::steady_clock::now();
    };
}