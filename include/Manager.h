#pragma once
#include <shared_mutex>
#include "Settings.h"
#include "Ticker.h"

struct PairFirstComparator {
    bool operator()(const std::pair<int, SaveSettings::Scenarios>& lhs,
                    const std::pair<int, SaveSettings::Scenarios>& rhs) const {
        return lhs.first < rhs.first;
    }
};

class Manager : public Ticker {
    
    std::shared_mutex sharedMutex_;

    std::set<std::pair<int, SaveSettings::Scenarios>, PairFirstComparator> queue;
    std::map<std::string, unsigned int> time_spent;

    
    void UpdateLoop();
    static void Init();
    bool SaveGame(SaveSettings::Scenarios reason);

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

    void QueueSaveGame(int seconds, SaveSettings::Scenarios scenario);

    std::vector<std::pair<int, SaveSettings::Scenarios>> GetQueue();

    bool DeleteQueuedSave(SaveSettings::Scenarios scenario);

    inline void ClearQueue();

    inline void QueueTimer();

};