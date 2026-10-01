#pragma once

namespace Utilities {

    const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());
    const auto plugin_version = SKSE::PluginDeclaration::GetSingleton()->GetVersion();

	std::filesystem::path GetLogPath();

	std::vector<std::string> ReadLogFile();

	inline bool GetCombatState() { return RE::PlayerCharacter::GetSingleton()->IsInCombat(); }

	void AutoSave(uint32_t flag = 0xf0000040);
    void QuitGame();

};

constexpr std::array bow_attack_states = {8, 10, 11, 13};
bool IsInBowAttackState(int attack_state);


