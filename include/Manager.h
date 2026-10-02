#pragma once
#include <shared_mutex>
#include "Settings.h"
#include "Ticker.h"

struct PairFirstComparator {
    bool operator()(const std::pair<int, SaveSettings::Scenarios>& lhs,
                    const std::pair<int, SaveSettings::Scenarios>& rhs) const {
        if (lhs.first != rhs.first) {
            return lhs.first < rhs.first;
        }
        return lhs.second < rhs.second;
    }
};

class Manager : public Ticker {
    
    std::shared_mutex sharedMutex_;
    std::uint64_t timerGeneration_ = 0;

    std::set<std::pair<int, SaveSettings::Scenarios>, PairFirstComparator> queue;
    std::map<std::string, unsigned int> time_spent;

    
    void UpdateLoop();
    static void Init();
    void SaveGame(SaveSettings::Scenarios reason, std::optional<std::uint64_t> timer_generation);
    void ExecuteSaveTask(SaveSettings::Scenarios reason, std::optional<std::uint64_t> timer_generation);

public:
    Manager()
        : Ticker([this]() { UpdateLoop(); }) {
        Init();
    }

    static Manager* GetSingleton() {
        static Manager singleton;
        return &singleton;
    }

    static void Uninstall();

    void DisableMod();

    static void EnableMod();

    void QueueSaveGame(int seconds, SaveSettings::Scenarios scenario,
                       std::optional<std::uint64_t> timer_generation = std::nullopt);

    std::vector<std::pair<int, SaveSettings::Scenarios>> GetQueue();

    bool DeleteQueuedSave(SaveSettings::Scenarios scenario);

    inline void ClearQueue();

    inline void QueueTimer(std::optional<std::uint64_t> timer_generation = std::nullopt);

};
