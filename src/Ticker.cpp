#include "Ticker.h"
#include "Settings.h"

void Ticker::Start() {
    if (m_Running) {
        return;
    }
    m_Running = true;
    logger::trace("Start Called with thread active state of: {}", m_ThreadActive);
    if (!m_ThreadActive) {
        std::thread tickerThread(&Ticker::RunLoop, this);
        tickerThread.detach();
    }
}

void Ticker::RunLoop(){
    m_ThreadActive = true;
    while (m_Running) {
        std::thread runnerThread(m_OnTick);
        runnerThread.detach();
        m_IntervalMutex.lock();
        m_Interval = std::chrono::milliseconds(std::min(60000, std::max(500, 1000*SaveSettings::ticker_interval)));
        m_IntervalMutex.unlock();
        std::this_thread::sleep_for(m_Interval);
    }
    m_ThreadActive = false;
};
