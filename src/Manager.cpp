#include "Manager.h"
#include <ranges>


void Manager::Uninstall() {}

void Manager::DisableMod(){
    PluginSettings::running = false;
    ClearQueue();
}
void Manager::EnableMod(){
    PluginSettings::running = true;
};

void Manager::QueueSaveGame(int seconds, SaveSettings::Scenarios scenario) {
    // mutex lock
    //std::lock_guard<std::mutex> lock(mutex);

    if (SaveSettings::block) return;
    if (!PluginSettings::running) return;

    // if Timer is queued, dont allow a second one
    if (scenario == SaveSettings::Scenarios::Timer) {
		for (const auto& snd : queue | std::views::values) {
			if (snd == SaveSettings::Scenarios::Timer) return;
		}
		SaveSettings::timer_running = true;
	}

    seconds = std::max(1, seconds);

    if (seconds > 0 && queue.size() < 100) {
        queue.insert(std::make_pair(seconds, scenario));
        const auto temp = std::format("Save queued for {} second(s).", seconds);
        if (SaveSettings::notifications && SaveSettings::queue_notif && scenario != SaveSettings::Scenarios::QuitGame) RE::DebugNotification(temp.c_str());
        Start();
    }
}

std::vector<std::pair<int, SaveSettings::Scenarios>> Manager::GetQueue() {
    // mutex lock
	std::shared_lock<std::shared_mutex> lock(sharedMutex_);
    return std::vector(queue.begin(), queue.end());
}

bool Manager::DeleteQueuedSave(const SaveSettings::Scenarios scenario){
    // mutex lock
	std::unique_lock<std::shared_mutex> lock(sharedMutex_);
    bool deleted = false;
	for (auto it = queue.begin(); it != queue.end();) {
		if (it->second == scenario) {
			it = queue.erase(it);
            deleted = true;
		} else ++it;
	}
    return deleted;
}

void Manager::ClearQueue(){
    Stop();
    // mutex lock
    std::unique_lock<std::shared_mutex> lock(sharedMutex_);
    queue.clear();
	SaveSettings::timer_running = false;
}
//inline bool Manager::IsInQueue(SaveSettings::Scenarios scenario)
//{
//	// mutex lock
//	std::shared_lock<std::shared_mutex> lock(sharedMutex_);
//	return std::ranges::any_of(queue, [scenario](const auto& pair) { return pair.second == scenario; });
//
//}
inline void Manager::QueueTimer()
{
	QueueSaveGame(SaveSettings::timer_minutes * 60 + SaveSettings::timer_seconds, SaveSettings::Scenarios::Timer);
};

void Manager::UpdateLoop() {

    if (m_Busy.exchange(true)) { 
        logger::trace("Busy flag set, returning...");
        return;
    }

    auto clearBusy = [](std::atomic<bool>* busyFlag) {
        //logger::trace("Clearing busy flag");
        busyFlag->store(false);
    };

    std::unique_ptr<std::atomic<bool>, decltype(clearBusy)> guard(&m_Busy, clearBusy);

    // mutex lock
    //std::lock_guard<std::mutex> lock(mutex);
    

    if (queue.empty()) {
        logger::trace("Queue is empty, stopping...");
        Stop();
        return;
    }
	if (const auto ui = RE::UI::GetSingleton();
        ui->GameIsPaused() || 
        ui->IsMenuOpen(RE::MainMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME) ||
        game_is_loading.load()) {
		logger::trace("Game is paused or menu is open, returning...");
		return;
	}

    // deduct time from all entries
    bool save = false;
    SaveSettings::Scenarios reason = {};
    const auto deduct = SaveSettings::ticker_interval;
    // unpack the queue to a vector
    std::vector<std::pair<int, SaveSettings::Scenarios>> queue_vector(queue.begin(), queue.end());
    for (auto it = queue_vector.begin(); it != queue_vector.end();) {
        it->first -= deduct;
        if (it->first <= 0) {
            reason = it->second;
            it = queue_vector.erase(it);
            save = true;
        } else ++it;
    }

    // repack the vector to the queue
    queue.clear();
    for (auto& entry : queue_vector) {
		queue.insert(entry);
	}

    if (save) {
        const bool saved = SaveGame(reason);
        if (reason == SaveSettings::Scenarios::Timer) {
            if (saved && SaveSettings::close_game) {
                RE::DebugNotification("Closing game in 10 seconds!");
                SaveSettings::close_game = false;
                QueueSaveGame(10, SaveSettings::Scenarios::QuitGame);
            }
            else if (SaveSettings::timer_periodic) {
				QueueTimer();
            }
            else SaveSettings::timer_running = false;
		    if (SaveSettings::close_game_warning) RE::DebugMessageBox("Time is up, close the game!");
        }
    }

}
void Manager::Init(){};

bool Manager::SaveGame(const SaveSettings::Scenarios reason) {
    if (reason == SaveSettings::Scenarios::QuitGame) {
		Utilities::QuitGame();
        return true;
	}
    //if (auto ui = RE::UI::GetSingleton(); ui && ui->GameIsPaused()) return QueueSaveGame(SaveSettings::queue_delay,reason);
    const auto ui = RE::UI::GetSingleton();
    if (!ui) {
        QueueSaveGame(SaveSettings::queue_delay, reason);
        return false;
    }
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        logger::error("PlayerCharacter is null!");
        return false;
    }
    const auto player_actorstate = player->AsActorState();
    if (!player_actorstate) {
        logger::error("PlayerCharacter ActorState is null!");
        return false;
    }
	const auto player_camera = RE::PlayerCamera::GetSingleton();
    const auto attack_state = static_cast<uint32_t>(player_actorstate->GetAttackState());
    if (SaveSettings::block || 
        !player->GetParentCell() ||
        player->IsDead() ||
        !player_camera ||
        player_camera->IsInBleedoutMode() ||
        player->IsOnMount() ||
        player->IsInCombat() ||
        player->IsInRagdollState() ||
        player->IsInKillMove() ||
        player->IsInMidair() ||
        IsInBowAttackState(attack_state) ||
        ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::MainMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::TweenMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::TrainingMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::StatsMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::SleepWaitMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::MapMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::MagicMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::LockpickingMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::LevelUpMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::JournalMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::GiftMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::FavoritesMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::BarterMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME) ||
        ui->IsMenuOpen(RE::TweenMenu::MENU_NAME)) {
        
        QueueSaveGame(SaveSettings::queue_delay, reason);
        return false;
    }
    
    logger::info("Saving game...");
    /*SKSE::GetTaskInterface()->AddTask([]() {*/
        //auto slm = RE::BGSSaveLoadManager::GetSingleton();
        //if (!slm) return;
        //if (slm->thread.isBusy) return;
        ////if (SaveSettings::freeze_game) GameLock::SetState(GameLock::State::Locked);
        //const auto player = RE::PlayerCharacter::GetSingleton();
        //if (!player) return;
        //auto cell = player->GetParentCell();
        //if (!cell) return;
        //const auto ws = player->GetWorldspace(); 
        //if (!ws) return;
        //const auto player_name = std::string(player->GetName());
        //const std::string date = "_" + std::format("{:%y%m%d%H%M%S}", std::chrono::system_clock::now());
        //auto player_parentcell = std::string(cell->GetFullName());
        //auto player_ws = std::string(ws->GetFullName());
        //if (!player_parentcell.empty()) player_parentcell = "_" + player_parentcell;
        //if (!player_ws.empty()) player_ws = "_" + player_ws;
        //slm->Save((player_name + player_ws + player_parentcell + date).c_str());
        //GameLock::SetState(GameLock::State::Unlocked);
	//});
    SKSE::GetTaskInterface()->AddTask(MainSaveFunction);
	return true;
}
