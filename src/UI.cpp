#include "UI.h"

void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


//std::vector<std::string> MCP::logLines;

void MCP::Register(Manager* manager) {
    
    M = manager;

    if (!SKSEMenuFramework::IsInstalled()) {
		logger::error("SKSEMenuFramework is not installed.");
        M->DisableMod();
		return;
	}

    SKSEMenuFramework::SetSection(Utilities::mod_name);
    SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);
    SKSEMenuFramework::AddSectionItem("Status", RenderStatus);
    SKSEMenuFramework::AddSectionItem("Log", RenderLog);

}

void __stdcall MCP::RenderSettings() {

    if (ImGui::Button("Save Settings")) SaveSettings::SaveJSON();
    ImGui::SameLine();
    ImGui::Checkbox("Auto Save", &SaveSettings::auto_save_to_json);
    ImGui::SameLine();
    HelpMarker("Save settings automatically upon game save");
    ImGui::SameLine();
    ImGui::Checkbox("Regular Saves", &SaveSettings::regular_saves);
    ImGui::SameLine();
    HelpMarker("Regular saves instead of auto saves.");

    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Min. Save Interval", &SaveSettings::temp_min_save_interval);
    SaveSettings::min_save_interval = std::max(0.0f, SaveSettings::temp_min_save_interval / 60.f);
    ImGui::SameLine();
    HelpMarker("Minimum allowed time between consecutive saves in in-game minutes");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180);
    if (ImGui::InputInt("Save Rotation Size", &SaveRegistry::max_saves)) {
        SaveRegistry::max_saves = std::min(std::max(0, SaveRegistry::max_saves),50);
		for (auto& [_, saves] : SaveRegistry::registry) {
			saves.rset_capacity(SaveRegistry::max_saves);
        }
		SaveRegistry::to_json();
    }
    ImGui::SameLine();
    HelpMarker("Maximum number of plugin-generated saves to keep before the oldest one is deleted.");

    ImGui::Checkbox("Notifications", &SaveSettings::notifications);
    ImGui::SameLine();
    HelpMarker("Show notifications when saving");
    ImGui::SameLine();
    ImGui::Checkbox("Save Queued Message", &SaveSettings::queue_notif);
    ImGui::SameLine();
    HelpMarker("Enable/Disable message displayed when a new save is queued.");

    ImGui::SameLine();

    // setting for blocking saves
    ImGui::Checkbox("Block Saves", &SaveSettings::block);
    ImGui::SameLine();
    HelpMarker("Block all saves");

    ImGui::SameLine();

    if (ImGui::Button("Clear")) M->ClearQueue();
    ImGui::SameLine();
    HelpMarker("Clear the save queue");

    ImGui::Text("");
    
    MCP::Settings::RenderTimer();

    ImGui::Text("");

    MCP::Settings::RenderCollapseExpandAll();

    // Default header state to true (open) if not already set
    for (const auto& header_name : header_names) {
        if (!headerStates.contains(header_name)) {
            headerStates[header_name] = true;
        }
    }

    // Render each header
    MCP::Settings::RenderMenu();

    MCP::Settings::RenderSleepWait();

    MCP::Settings::RenderCombat();

    MCP::Settings::RenderMisc();
};

void MCP::Settings::RenderCollapseExpandAll() {
    // checkbox for collapse all below
    if (ImGui::Button("Collapse All")) {
        for (const auto& header_name : header_names) {
            headerStates[header_name] = false;
        }
    }

    // checkbox for expand all below
    ImGui::SameLine();
    if (ImGui::Button("Expand All")) {
        for (const auto& header_name : header_names) {
            headerStates[header_name] = true;
        }
    }
}

void MCP::Settings::RenderTimer(){
    
    // Add timer to save game after x minutes with Start/Stop/Reset buttons
    
    // show remaining time in seconds, where we dont know if the timer is running or not or if it is the first
    // element in the queue
    int temp_remaining = 0;
    for (const auto& [t, reason] : M->GetQueue()) {
        if (reason == SaveSettings::Scenarios::Timer) {
            temp_remaining = t;
            break;
        }
    }

    if (ImGui::CollapsingHeader("Timer##header", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Periodic", &SaveSettings::timer_periodic);
        ImGui::SameLine();
        ImGui::Text("Minutes:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(180);
        ImGui::InputInt("##minutes", &SaveSettings::timer_minutes);
        ImGui::SameLine();
        ImGui::Text("Seconds:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(180);
        ImGui::InputInt("##timer_seconds", &SaveSettings::timer_seconds);
        ImGui::SameLine();
        if (ImGui::Button("Start##timer")) {
			M->QueueTimer();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop##timer")) {
            M->DeleteQueuedSave(SaveSettings::Scenarios::Timer);
            SaveSettings::timer_running = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset##timer")) {
            if (M->DeleteQueuedSave(SaveSettings::Scenarios::Timer)) {
				M->QueueTimer();
            } else {
                SaveSettings::timer_minutes = 0;
                SaveSettings::timer_seconds = 0;
                SaveSettings::timer_running = false;
            }
        }
        ImGui::SameLine();
        ImGui::Text("Remaining: %d", temp_remaining);

        // add checkbox for warning when time is up
        ImGui::Checkbox("Close Game Warning", &SaveSettings::close_game_warning);
        ImGui::SameLine();
        HelpMarker("Show warning when time is up");
        // add checkbox for closing the game when time is up
        ImGui::SameLine();
        ImGui::Checkbox("Close Game", &SaveSettings::close_game);
        ImGui::SameLine();
        HelpMarker("Close the game when time is up");
    }
};

void MCP::Settings::RenderMenu() {

    if (!headerStates["Menu"]) ImGui::SetNextItemOpen(false);
    else ImGui::SetNextItemOpen(true);
    if (ImGui::CollapsingHeader("Menu", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Reset All##Menu")) {
            for (auto& [menu_name, _] : SaveSettings::Menu::Open) {
			    SaveSettings::Menu::Open[menu_name].first = false;
			    SaveSettings::Menu::Close[menu_name].first = false;
			    SaveSettings::Menu::After[menu_name] = 4;
                SaveSettings::Menu::MinTimeSpent[menu_name] = 10;
		    }
        }
        headerStates["Menu"] = true;

        constexpr float maxTextWidth = 180.0f;
        //for (const auto& [menu_name, _] : SaveSettings::Menu::Open) {
            //maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(menu_name.c_str()).x);
        //}

        for (auto& [menu_name, _] : SaveSettings::Menu::Open) {
            bool setting_open = SaveSettings::Menu::Open[menu_name].first;
            bool setting_close = SaveSettings::Menu::Close[menu_name].first;
            int setting_after = SaveSettings::Menu::After[menu_name];
            int setting_time_spent = SaveSettings::Menu::MinTimeSpent[menu_name];
            
            ImGui::Text(menu_name.c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(maxTextWidth + 20);  // Adjust the 20 value to set spacing
            std::string checkbox_name_open = "Open##" + menu_name;
            ImGui::Checkbox(checkbox_name_open.c_str(), &setting_open);
            ImGui::SameLine();
            std::string checkbox_name_close = "Close##" + menu_name;
            ImGui::Checkbox(checkbox_name_close.c_str(), &setting_close);
            // add text with field for seconds to wait after menu is closed
            ImGui::SameLine();
            ImGui::Text("After");
            ImGui::SameLine();
            std::string input_name = "##" + menu_name;
            ImGui::SetNextItemWidth(180);
            ImGui::InputInt(input_name.c_str(), &setting_after);
            ImGui::SameLine();
            HelpMarker("Seconds to wait after menu is closed before saving");

            // add text with field for minimum time spent in the menu before saving
            ImGui::SameLine();
            ImGui::Text("Min. Time Spent");
            ImGui::SameLine();
            std::string input_name_time_spent = "##" + menu_name + "_time_spent";
            ImGui::SetNextItemWidth(180);
            ImGui::InputInt(input_name_time_spent.c_str(), &setting_time_spent);
            ImGui::SameLine();
            HelpMarker("Minimum time spent (seconds) in the menu before saving after closing");
            
            SaveSettings::Menu::Open[menu_name].first = setting_open;
            SaveSettings::Menu::Close[menu_name].first = setting_close;
            SaveSettings::Menu::After[menu_name] = setting_after;
            SaveSettings::Menu::MinTimeSpent[menu_name] = setting_time_spent;
        };
    } else headerStates["Menu"] = false;
}

void MCP::Settings::RenderSleepWait(){

    if (!headerStates["SleepWait"]) ImGui::SetNextItemOpen(false);
    else ImGui::SetNextItemOpen(true);
    if (ImGui::CollapsingHeader("Sleep/Wait", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Reset All##sleepwait")) {
			SaveSettings::SleepWait::sleep = false;
			SaveSettings::SleepWait::wait = false;
			SaveSettings::SleepWait::sleep_time = 0;
			SaveSettings::SleepWait::wait_time = 0;
        }
        headerStates["SleepWait"] = true;

        constexpr float maxTextWidth = 180.0f;
        //for (const auto& temp_name : {"Sleep", "Wait"}) {
            //maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(temp_name).x);
        //}
            
        ImGui::Checkbox("Sleep", &SaveSettings::SleepWait::sleep);
        ImGui::SameLine();
        ImGui::SetCursorPosX(maxTextWidth + 20);  // Adjust the 20 value to set spacing
        ImGui::Text("After");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(180);
        ImGui::InputInt("##sleep_time", &SaveSettings::SleepWait::sleep_time);
        ImGui::SameLine();
        HelpMarker("Seconds to save after waking up");

        ImGui::Checkbox("Wait", &SaveSettings::SleepWait::wait);
        ImGui::SameLine();
        ImGui::SetCursorPosX(maxTextWidth + 20);  // Adjust the 20 value to set spacing
        ImGui::Text("After");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(180);
        ImGui::InputInt("##wait_time", &SaveSettings::SleepWait::wait_time);
        ImGui::SameLine();
        HelpMarker("Seconds to save after waiting");
    } else headerStates["SleepWait"] = false;
    
}
void MCP::Settings::RenderCombat(){
    if (!headerStates["Combat"]) ImGui::SetNextItemOpen(false);
    else ImGui::SetNextItemOpen(true);
    if (ImGui::CollapsingHeader("Combat##header", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Reset All##Combat")) {
            SaveSettings::Combat::entering_combat = false;
            SaveSettings::Combat::exiting_combat = false;
            SaveSettings::Combat::combat_time = 0;
            SaveSettings::Combat::min_combat_time_exit = 0;
        }
        headerStates["Combat"] = true;

        constexpr float maxTextWidth = 180.0f;

        bool setting_open = SaveSettings::Combat::entering_combat;
        bool setting_close = SaveSettings::Combat::exiting_combat;
        int setting_after = SaveSettings::Combat::combat_time;
        int setting_time_spent = SaveSettings::Combat::min_combat_time_exit;

        const std::string menu_name = "CombatEvents";
        ImGui::Text(menu_name.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(maxTextWidth + 20);  // Adjust the 20 value to set spacing
        const std::string checkbox_name_open = "Enter##" + menu_name;
        ImGui::Checkbox(checkbox_name_open.c_str(), &setting_open);
        ImGui::SameLine();
        const std::string checkbox_name_close = "Exit##" + menu_name;
        ImGui::Checkbox(checkbox_name_close.c_str(), &setting_close);
        // add text with field for seconds to wait after menu is closed
        ImGui::SameLine();
        ImGui::Text("After");
        ImGui::SameLine();
        const std::string input_name = "##" + menu_name;
        ImGui::SetNextItemWidth(180);
        ImGui::InputInt(input_name.c_str(), &setting_after);
        ImGui::SameLine();
        HelpMarker("Seconds to wait after leaving combat before saving");

        // add text with field for minimum time spent in the menu before saving
        ImGui::SameLine();
        ImGui::Text("Min. Time Spent");
        ImGui::SameLine();
        std::string input_name_time_spent = "##" + menu_name + "_time_spent";
        ImGui::SetNextItemWidth(180);
        ImGui::InputInt(input_name_time_spent.c_str(), &setting_time_spent);
        ImGui::SameLine();
        HelpMarker("Minimum time spent (seconds) in combat before saving after exiting");

        SaveSettings::Combat::entering_combat = setting_open;
        SaveSettings::Combat::exiting_combat = setting_close;
        SaveSettings::Combat::combat_time = setting_after;
        SaveSettings::Combat::min_combat_time_exit = setting_time_spent;
    } else headerStates["Combat"] = false;
}
void MCP::Settings::RenderMisc(){
    if (!headerStates["Misc"])
        ImGui::SetNextItemOpen(false);
    else
        ImGui::SetNextItemOpen(true);
    if (ImGui::CollapsingHeader("Misc##header", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Reset All##Misc")) {
            SaveSettings::Other::save_on_level_up = false;
        }
        headerStates["Misc"] = true;

        constexpr float maxTextWidth = 180.0f;

        ImGui::Text("LevelUp");
        ImGui::SameLine();
        ImGui::SetCursorPosX(maxTextWidth + 20);  // Adjust the 20 value to set spacing
        ImGui::Checkbox("##LevelUpCheckBox", &SaveSettings::Other::save_on_level_up);
        ImGui::SameLine();
        HelpMarker("Save on level up");

        ImGui::Text("SkillUp");
        ImGui::SameLine();
        ImGui::SetCursorPosX(maxTextWidth + 20);  // Adjust the 20 value to set spacing
        ImGui::Checkbox("##SkillUpCheckBox", &SaveSettings::Other::save_on_skill_up);
        ImGui::SameLine();
        HelpMarker("Save when a skill's point increases");

        ImGui::Text("QuestComplete");
        ImGui::SameLine();
        ImGui::SetCursorPosX(maxTextWidth + 20);  // Adjust the 20 value to set spacing
        ImGui::Checkbox("##QuestCompleteCheckBox", &SaveSettings::Other::save_on_quest_complete);
        ImGui::SameLine();
        HelpMarker("Save when a quest is completed");

        ImGui::Text("LocationDiscover");
        ImGui::SameLine();
        ImGui::SetCursorPosX(maxTextWidth + 20);  // Adjust the 20 value to set spacing
        ImGui::Checkbox("##LocationDiscoverCheckBox", &SaveSettings::Other::save_on_location_discover);
        ImGui::SameLine();
        HelpMarker("Save when a location is discovered");

    } else headerStates["Misc"] = false;
};

void __stdcall MCP::RenderStatus(){
    // Status of the plugin with color green if running, red if stopped
    ImGui::Text("Status: ");
    ImGui::SameLine();
    if (PluginSettings::running) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Running");
	} else {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Stopped");
	}

    // add disable/enable button if enabled/disabled

    if (PluginSettings::running) {
		if (ImGui::Button("Disable Mod")) {
			M->DisableMod();
		}
	} else {
		if (ImGui::Button("Enable Mod")) {
            Manager::EnableMod();
		}
	}

    // add refresh button to reload the queue

    //FontAwesome::PushSolid();
    //if (ImGui::Button((FontAwesome::UnicodeToUtf8(0xf021) + " Refresh").c_str())) {
    //    queue_cache = M->GetQueue();
    //}
    //FontAwesome::Pop();
    // display the queue in table format
    ImGui::Columns(2, "Queue", true);
    ImGui::Text("Time");
    ImGui::NextColumn();
    ImGui::Text("Reason");
    ImGui::NextColumn();
    ImGui::Separator();

    for (const auto& [t, reason] : M->GetQueue()) {
        ImGui::Text("%d", t);
		ImGui::NextColumn();
        if (!SaveSettings::scenario_names.contains(reason)) ImGui::Text("Unknown");
        else ImGui::Text(SaveSettings::scenario_names[reason].c_str());
		ImGui::NextColumn();
	}
};

void __stdcall MCP::RenderLog() {

    // add checkboxes to filter log levels
    ImGui::Checkbox("Trace", &LogSettings::log_trace);
    ImGui::SameLine();
    ImGui::Checkbox("Info", &LogSettings::log_info);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &LogSettings::log_warning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &LogSettings::log_error);


    // if "Generate Log" button is pressed, read the log file
    if (ImGui::Button("Generate Log")) {
		logLines = Utilities::ReadLogFile();
	}

    // Display each line in a new ImGui::Text() element
    for (const auto& line : logLines) {
        if (line.find("trace") != std::string::npos && !LogSettings::log_trace) continue;
        if (line.find("info") != std::string::npos && !LogSettings::log_info) continue;
        if (line.find("warning") != std::string::npos && !LogSettings::log_warning) continue;
        if (line.find("error") != std::string::npos && !LogSettings::log_error) continue;
		ImGui::Text(line.c_str());
	}
    
}


