#include <SKSE/SKSE.h>
#include "TheLastBreath/AnimationHandler.h"
#include "TheLastBreath/CombatEventHandler.h"
#include "TheLastBreath/Config.h"
#include "TheLastBreath/Hooks.h"
#include "TheLastBreath/RangedStaminaHandler.h"
#include "TheLastBreath/ExhaustionHandler.h"
#include "TheLastBreath/HitEventHandler.h"
#include "TheLastBreath/CombatHandler.h"
#include "TheLastBreath/Data.h"
#include "TheLastBreath/EldenCounterCompat.h"
#include <atomic>
#include <thread>

using namespace SKSE;
using namespace SKSE::log;
using namespace std::chrono_literals;

namespace {
    constexpr const char* PLUGIN_NAME = "TheLastBreath";
    constexpr const char* PLUGIN_AUTHOR = "Heisen";
    constexpr REL::Version PLUGIN_VERSION = { 1, 0, 0, 0 };
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginName(PLUGIN_NAME);
    v.AuthorName(PLUGIN_AUTHOR);
    v.PluginVersion(PLUGIN_VERSION);
    v.UsesAddressLibrary();
    v.UsesNoStructs();
    return v;
    }();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
    pluginInfo->name = SKSEPlugin_Version.pluginName;
    pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
    pluginInfo->version = SKSEPlugin_Version.pluginVersion;
    return true;
}

namespace {
    std::atomic<bool> g_registered = false;
    std::atomic<bool> g_gameLoaded = false;

    // Worker thread for continuous updates
    static std::atomic_bool g_updateWorkerRunning{ false };
    static std::thread g_updateWorker;

    static void StartUpdateWorker()
    {
        if (g_updateWorkerRunning.exchange(true)) return;

        g_updateWorker = std::thread([]() {
            while (g_updateWorkerRunning.load(std::memory_order_relaxed)) {
                TheLastBreath::RangedStaminaHandler::GetSingleton()->Update();
                TheLastBreath::ExhaustionHandler::GetSingleton()->Update();
                TheLastBreath::TimedBlockHandler::GetSingleton()->Update();
                TheLastBreath::CombatHandler::GetSingleton()->Update();
                std::this_thread::sleep_for(100ms);
            }
            });
    }

    static void StopUpdateWorker()
    {
        if (!g_updateWorkerRunning.exchange(false)) return;
        if (g_updateWorker.joinable()) g_updateWorker.join();
    }

    class InputEventHandler : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        static InputEventHandler* GetSingleton() {
            static InputEventHandler singleton;
            return &singleton;
        }

        // Helper function to check if player can block (has weapon or shield)
        static bool CanPlayerBlock(RE::PlayerCharacter* player) {
            if (!player) return false;

            // Check right hand (weapon slot)
            auto rightHand = player->GetEquippedObject(false);
            if (rightHand) {
                auto weapon = rightHand->As<RE::TESObjectWEAP>();
                if (weapon) {
                    // Player has a weapon equipped - can block
                    return true;
                }
            }

            // Check left hand (shield/weapon slot)
            auto leftHand = player->GetEquippedObject(true);
            if (leftHand) {
                // Check if it's armor (shield)
                auto armor = leftHand->As<RE::TESObjectARMO>();
                if (armor) {
                    // Shields are armor, so if we have armor equipped in left hand, it's a shield
                    return true;
                }
                // Check if it's a weapon (dual wielding)
                auto weapon = leftHand->As<RE::TESObjectWEAP>();
                if (weapon) {
                    // Player has a weapon in left hand (dual wielding) - can block
                    return true;
                }
            }

            // No weapon or shield equipped - cannot block
            return false;
        }

        virtual RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* a_event,
            RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override
        {
            if (!a_event) {
                return RE::BSEventNotifyControl::kContinue;
            }

            auto config = TheLastBreath::Config::GetSingleton();
            auto player = RE::PlayerCharacter::GetSingleton();

            // Device offsets for universal key codes
            constexpr uint32_t kKeyboardOffset = 0;
            constexpr uint32_t kMouseOffset = 256;
            constexpr uint32_t kGamepadOffset = 266;

            if (player && config->enableTimedBlocking) {
                for (auto event = *a_event; event; event = event->next) {
                    if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) {
                        auto buttonEvent = event->AsButtonEvent();
                        if (buttonEvent) {
                            auto device = buttonEvent->GetDevice();
                            uint32_t keyCode = buttonEvent->GetIDCode();

                            // Convert to universal key code using device offset
                            switch (device) {
                            case RE::INPUT_DEVICE::kMouse:
                                keyCode += kMouseOffset;
                                break;
                            case RE::INPUT_DEVICE::kKeyboard:
                                keyCode += kKeyboardOffset;
                                break;
                            case RE::INPUT_DEVICE::kGamepad:
                                keyCode += kGamepadOffset;
                                break;
                            default:
                                continue;
                            }

                            // Check if this is our configured block button
                            if (keyCode == config->blockButton) {
                                if (!CanPlayerBlock(player)) {
                                    continue;
                                }

                                if (buttonEvent->IsDown() && buttonEvent->value > 0.0f) {
                                    logger::debug("Block button PRESSED (key: {})", keyCode);
                                    TheLastBreath::TimedBlockHandler::GetSingleton()->OnButtonPressed(player);
                                    TheLastBreath::CombatHandler::GetSingleton()->OnBlockStart(player);
                                }
                                else if (buttonEvent->IsUp() || buttonEvent->value == 0.0f) {
                                    logger::debug("Block button RELEASED (key: {})", keyCode);
                                    TheLastBreath::TimedBlockHandler::GetSingleton()->OnButtonReleased(player);
                                    TheLastBreath::CombatHandler::GetSingleton()->OnBlockStop(player);
                                }
                            }
                        }
                    }
                }
            }


            // Original animation registration logic
            if (g_registered.load()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (!g_gameLoaded.load()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (g_registered.exchange(true)) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (player) {
                bool result = player->AddAnimationGraphEventSink(TheLastBreath::AnimationEventHandler::GetSingleton());
                if (result) {
                    logger::debug("Animation events registered for player");
                }
                else {
                    g_registered.store(false);
                }
            }
            else {
                g_registered.store(false);
            }

            return RE::BSEventNotifyControl::kContinue;
        }

    private:
        InputEventHandler() = default;
        InputEventHandler(const InputEventHandler&) = delete;
        InputEventHandler(InputEventHandler&&) = delete;
    };

    void InitializeLog() {
        auto path = log_directory();
        if (!path) return;

        *path /= "TheLastBreath.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

        // Load INI config early
        auto config = TheLastBreath::Config::GetSingleton();
        config->Load();

        auto level = static_cast<spdlog::level::level_enum>(config->logLevel);
        log->set_level(level);
        log->flush_on(level);

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%H:%M:%S] [%l] %v");
    }

    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
        {
            logger::debug("kDataLoaded message received");

            // Load all game data (sounds, FX, etc.)
            TheLastBreath::Data::LoadData();

            logger::info("Configuration loaded");

            // Register event handlers
            if (auto inputManager = RE::BSInputDeviceManager::GetSingleton()) {
                inputManager->AddEventSink(InputEventHandler::GetSingleton());
                logger::debug("Input event handler registered");
            }
            else {
                logger::error("Failed to get input device manager");
            }

            if (auto scriptEventSource = RE::ScriptEventSourceHolder::GetSingleton()) {
                scriptEventSource->AddEventSink(TheLastBreath::CombatEventHandler::GetSingleton());
                logger::debug("Combat event handler registered for NPC tracking");

                scriptEventSource->AddEventSink(TheLastBreath::HitEventHandler::GetSingleton());
                logger::debug("Hit event handler registered");
            }
            else {
                logger::error("Failed to get script event source");
            }

            // Initialize Elden Counter compatibility
            TheLastBreath::EldenCounterCompat::GetSingleton()->Initialize();

            break;
        }

        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame:
        {
            logger::debug("kPostLoadGame/kNewGame message received");

            g_registered.store(false);
            g_gameLoaded.store(true);

            TheLastBreath::ExhaustionHandler::GetSingleton()->ClearAll();
            logger::debug("Ready - animation events will register on first player input");

            StartUpdateWorker();

            break;
        }

        case SKSE::MessagingInterface::kPreLoadGame:
        case SKSE::MessagingInterface::kDeleteGame:
        {
            StopUpdateWorker();
            break;
        }

        }
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    InitializeLog();
    logger::info("{} v{} loading...", PLUGIN_NAME, PLUGIN_VERSION.string());

    SKSE::Init(a_skse);

    TheLastBreath::Hooks::Install();

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener(MessageHandler)) {
        logger::critical("Failed to register message listener");
        return false;
    }

    logger::info("{} loaded successfully", PLUGIN_NAME);
    return true;
}