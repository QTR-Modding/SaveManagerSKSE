#pragma once
#include "Settings.h"


// https://github.com/ozooma10/OSLAroused-SKSE/blob/master/src/Utilities/Ticker.h
class Ticker {
public:
    explicit Ticker(const std::function<void()>& onTick) : m_OnTick(onTick), m_ThreadActive(false), m_Running(false) {
        m_Interval = std::chrono::milliseconds(SaveSettings::ticker_interval*1000);
    }

    void Start();

    void Stop() { m_Running = false; }

    std::atomic<bool> m_Busy;

private:
    void RunLoop();

    std::function<void()> m_OnTick;
    std::chrono::milliseconds m_Interval;

    std::atomic<bool> m_ThreadActive;
    std::atomic<bool> m_Running;
    std::mutex m_IntervalMutex;
};