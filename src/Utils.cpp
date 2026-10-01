#include "Utils.h"

std::filesystem::path Utilities::GetLogPath() {
    const auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    return logFilePath;
}

std::vector<std::string> Utilities::ReadLogFile() {
    std::vector<std::string> logLines;

    // Open the log file
    std::ifstream file(GetLogPath().c_str());
    if (!file.is_open()) {
        // Handle error
        return logLines;
    }

    // Read and store each line from the file
    std::string line;
    while (std::getline(file, line)) {
        logLines.push_back(line);
    }

    file.close();

    return logLines;
}

void Utilities::AutoSave(uint32_t flag) {
    // Credits: SkyrimThiago
    // auto save 0xf0000040
    // regular save 0xf0000080
    using func_t = void(RE::BGSSaveLoadManager * a1, uint32_t a2);
    REL::Relocation<func_t> func{RELOCATION_ID(34859, 35769)};
    return func(RE::BGSSaveLoadManager::GetSingleton(), flag);
}
void Utilities::QuitGame(){
    SKSE::GetTaskInterface()->AddTask([]() { RE::Main::GetSingleton()->GetRuntimeData().quitGame = true; });
};

bool IsInBowAttackState(const int attack_state) { 

    if (std::ranges::find(bow_attack_states, attack_state) != std::end(bow_attack_states)) {
        return true;
    }
    return false;
}

