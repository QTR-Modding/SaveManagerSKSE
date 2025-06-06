#pragma once

#include "rapidjson/document.h"
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

//#include <chrono>
#include <windows.h>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include "SimpleIni.h"
#include <iostream>
#include <string>
#include <codecvt>
#include <mutex>
#include <algorithm>
#include <ClibUtil/editorID.hpp>
#include <filesystem>
#include <fstream>
#include <boost/circular_buffer.hpp>

namespace Utilities {

    const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());
    const auto plugin_version = SKSE::PluginDeclaration::GetSingleton()->GetVersion();

	std::filesystem::path GetLogPath();

	std::vector<std::string> ReadLogFile();

	inline bool GetCombatState() { return RE::PlayerCharacter::GetSingleton()->IsInCombat(); }

	void AutoSave(uint32_t flag = 0xf0000040);
    void QuitGame();

};

constexpr std::array<int, 4> bow_attack_states = {8, 10, 11, 13};
bool IsInBowAttackState(const int attack_state);


