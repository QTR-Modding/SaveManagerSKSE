#include "Events.h"

RE::BSEventNotifyControl ourEventSink::ProcessEvent(const RE::TESContainerChangedEvent*,
                                                    RE::BSTEventSource<RE::TESContainerChangedEvent>*) {
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl ourEventSink::ProcessEvent(const RE::TESFurnitureEvent*,
                                                    RE::BSTEventSource<RE::TESFurnitureEvent>*) {
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl ourEventSink::ProcessEvent(const RE::MenuOpenCloseEvent* event,
													RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (!event) return RE::BSEventNotifyControl::kContinue;
    const auto menu_name = std::string(event->menuName.c_str());
    if (!SaveSettings::Menu::Open.contains(menu_name)) return RE::BSEventNotifyControl::kContinue;
    if (event->opening) {
        // record the time the menu was opened
        menu_times[menu_name] = datetime::now();
        return RE::BSEventNotifyControl::kContinue;
    }

    if (const auto temp_setting_close = SaveSettings::Menu::Close[menu_name]; temp_setting_close.first) {
        const auto min_time_spent = SaveSettings::Menu::MinTimeSpent[menu_name];
        if (min_time_spent > 0) {
			const auto time_spent = std::chrono::duration_cast<std::chrono::seconds>(datetime::now() - menu_times[menu_name]).count();
			if (time_spent < min_time_spent) return RE::BSEventNotifyControl::kContinue;
		}
        M->QueueSaveGame(SaveSettings::Menu::After[menu_name], temp_setting_close.second);
        
        menu_times.erase(menu_name);
		
        return RE::BSEventNotifyControl::kContinue;
	}

	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl ourEventSink::ProcessEvent(const RE::TESSleepStopEvent* event,
                                                    RE::BSTEventSource<RE::TESSleepStopEvent>*) {
    if (!event) return RE::BSEventNotifyControl::kContinue;
	if (!SaveSettings::SleepWait::sleep) return RE::BSEventNotifyControl::kContinue;

    M->QueueSaveGame(SaveSettings::SleepWait::sleep_time, SaveSettings::Scenarios::SleepWaitStop);

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl ourEventSink::ProcessEvent(const RE::TESWaitStopEvent* event,
                                                    RE::BSTEventSource<RE::TESWaitStopEvent>*) {
    if (!event) return RE::BSEventNotifyControl::kContinue;
    if (!SaveSettings::SleepWait::wait) return RE::BSEventNotifyControl::kContinue;

    M->QueueSaveGame(SaveSettings::SleepWait::wait_time, SaveSettings::Scenarios::SleepWaitStop);

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl ourEventSink::ProcessEvent(const RE::TESCombatEvent* event,
                                                    RE::BSTEventSource<RE::TESCombatEvent>*) {
    
    if (!event) return RE::BSEventNotifyControl::kContinue;

    if (const auto combat_state = event->newState.underlying(); combat_state != 0 && in_combat) return RE::BSEventNotifyControl::kContinue;
    else if (combat_state == 0 && !in_combat) return RE::BSEventNotifyControl::kContinue;
    else if (combat_state == 0 && in_combat) {
        in_combat = Utilities::GetCombatState();
        if (!in_combat) {
            const auto time_spent = std::chrono::duration_cast<std::chrono::seconds>(datetime::now() - menu_times["Combat"]).count();
            menu_times.erase("Combat");
            if (time_spent < SaveSettings::Combat::min_combat_time_exit) return RE::BSEventNotifyControl::kContinue;
            if (SaveSettings::Combat::exiting_combat) M->QueueSaveGame(SaveSettings::Combat::combat_time, SaveSettings::Scenarios::CombatExit);
        };

        return RE::BSEventNotifyControl::kContinue;
    };

    if (const auto* event_target = event->targetActor.get(); !event_target || !event_target->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;

    if (!in_combat) {
		in_combat = true;
        menu_times["Combat"] = datetime::now();
		if (SaveSettings::Combat::entering_combat) MainSaveFunction();
	}


    
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl ourEventSink::ProcessEvent(const RE::TESTrackedStatsEvent* event,
                                                    RE::BSTEventSource<RE::TESTrackedStatsEvent>*) {
    if (!event) return RE::BSEventNotifyControl::kContinue;
    
    if (_strcmpi(event->stat.c_str(), "Level Increases") == 0 && SaveSettings::Other::save_on_level_up) {
        M->QueueSaveGame(SaveSettings::queue_delay, SaveSettings::Scenarios::LevelUp);
    } 
    else if (_strcmpi(event->stat.c_str(), "Skill Increases") == 0 && SaveSettings::Other::save_on_skill_up) {
        M->QueueSaveGame(SaveSettings::queue_delay, SaveSettings::Scenarios::SkillUp);
    } 
    else if (_strcmpi(event->stat.c_str(), "Quests Completed") == 0 && SaveSettings::Other::save_on_quest_complete) {
        M->QueueSaveGame(SaveSettings::queue_delay, SaveSettings::Scenarios::QuestComplete);
    } 
    else if (_strcmpi(event->stat.c_str(), "Locations Discovered") == 0 && SaveSettings::Other::save_on_location_discover) {
        M->QueueSaveGame(SaveSettings::queue_delay, SaveSettings::Scenarios::LocationDiscover);
    }
    
    return RE::BSEventNotifyControl::kContinue;
}
