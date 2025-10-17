#include <SKSE/SKSE.h>
#include "TheLastBreath/AnimationHandler.h"
#include "TheLastBreath/CombatEventHandler.h"  
#include "TheLastBreath/SlowMotion.h"
#include "TheLastBreath/Config.h"
#include "TheLastBreath/Hooks.h"
#include "TheLastBreath/RangedStaminaHandler.h"
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

    static std::atomic_bool g_rangedWorkerRunning{ false };
    static std::thread      g_rangedWorker;

    static void StartRangedStaminaWorker()
    {
        if (g_rangedWorkerRunning.exchange(true)) return;

        g_rangedWorker = std::thread([]() {
            while (g_rangedWorkerRunning.load(std::memory_order_relaxed)) {
                SKSE::GetTaskInterface()->AddTask([]() {
                    TheLastBreath::RangedStaminaHandler::GetSingleton()->Update();
                    });
                std::this_thread::sleep_for(100ms);
            }
            });
    }

    static void StopRangedStaminaWorker()
    {
        if (!g_rangedWorkerRunning.exchange(false)) return;
        if (g_rangedWorker.joinable()) g_rangedWorker.join();
    }

    class InputEventHandler : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        static InputEventHandler* GetSingleton() {
            static InputEventHandler singleton;
            return &singleton;
        }

        virtual RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* a_event,
            RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override
        {
            if (g_registered.load()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (!g_gameLoaded.load()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (!a_event) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (g_registered.exchange(true)) {
                return RE::BSEventNotifyControl::kContinue;
            }

            auto player = RE::PlayerCharacter::GetSingleton();
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

            TheLastBreath::RangedStaminaHandler::GetSingleton()->Update();

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
            logger::info("Configuration loaded");

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
            }
            else {
                logger::error("Failed to get script event source");
            }

            if (auto ui = RE::UI::GetSingleton()) {
                ui->AddEventSink(TheLastBreath::RangedStaminaHandler::GetSingleton());
                logger::debug("Ranged stamina update handler registered");
            }
            else {
                logger::error("Failed to get UI singleton");
            }

            break;
        }

        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame:
        {
            logger::debug("kPostLoadGame/kNewGame message received");

            g_registered.store(false);
            g_gameLoaded.store(true);

            StartRangedStaminaWorker();

            TheLastBreath::SlowMotionManager::GetSingleton()->ClearAll();
            logger::debug("Ready - animation events will register on first player input");

            break;
        }

        case SKSE::MessagingInterface::kPreLoadGame:
        case SKSE::MessagingInterface::kDeleteGame:
        {
            StopRangedStaminaWorker();
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
