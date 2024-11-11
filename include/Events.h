#pragma once

#include "Manager.h"

class ourEventSink final : public RE::BSTEventSink<RE::TESContainerChangedEvent>,
                           public RE::BSTEventSink<RE::TESFurnitureEvent>,
                           public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                           public RE::BSTEventSink<RE::TESSleepStopEvent>,
                           public RE::BSTEventSink<RE::TESWaitStopEvent>,
                           public RE::BSTEventSink<RE::TESCombatEvent>,
                           public RE::BSTEventSink<RE::TESTrackedStatsEvent>
{
public:

    ourEventSink() = default;
    ourEventSink(const ourEventSink&) = delete;
    ourEventSink(ourEventSink&&) = delete;
    ourEventSink& operator=(const ourEventSink&) = delete;
    ourEventSink& operator=(ourEventSink&&) = delete;


    static ourEventSink* GetSingleton(Manager* manager) {
        static ourEventSink singleton(manager);
        return &singleton;
    }

    bool in_combat = false;

    [[maybe_unused]] RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent*,
                                          RE::BSTEventSource<RE::TESContainerChangedEvent>*) override;

    [[maybe_unused]] RE::BSEventNotifyControl ProcessEvent(const RE::TESFurnitureEvent*,
                                          RE::BSTEventSource<RE::TESFurnitureEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESSleepStopEvent* event, RE::BSTEventSource<RE::TESSleepStopEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESWaitStopEvent* event, RE::BSTEventSource<RE::TESWaitStopEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESTrackedStatsEvent* event,
                                          RE::BSTEventSource<RE::TESTrackedStatsEvent>*) override;

private:
    Manager* M = nullptr;

    std::map<std::string, datetime::time_point> menu_times;


    explicit ourEventSink(Manager* manager) : M(manager){ 
        if (SaveSettings::timer_periodic && SaveSettings::timer_running) {
			M->QueueTimer();
		}
    }
};