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
        std::this_thread::sleep_for(std::chrono::seconds(SaveSettings::ticker_interval));
        if (!m_Running) break;
        std::thread runnerThread(m_OnTick);
        runnerThread.detach();
    }
    m_ThreadActive = false;
};
