#include "Hooks.h"

using namespace Hooks;

template <typename MenuType>
void MenuHook<MenuType>::InstallHook(const REL::VariantID& varID) {
    REL::Relocation<std::uintptr_t> vTable(varID);
    _ProcessMessage = vTable.write_vfunc(0x4, &MenuHook<MenuType>::ProcessMessage_Hook);
}

template <typename MenuType>
RE::UI_MESSAGE_RESULTS MenuHook<MenuType>::ProcessMessage_Hook(RE::UIMessage& a_message) {
    if (const std::string_view menuName = MenuType::MENU_NAME; _strcmpi(a_message.menu.c_str(), std::string(menuName).c_str()) == 0) {

        if (auto _menu = RE::UI::GetSingleton()->GetMenu<MenuType>(menuName)) {
            if (const auto msg_type = static_cast<int>(a_message.type.get());
                msg_type == 1 && SaveSettings::Menu::Open.contains(std::string(menuName)) &&
                SaveSettings::Menu::Open[std::string(menuName)].first) {
                const auto old_flag = _menu->menuFlags.get();
                _menu->menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving);
                MainSaveFunction();
                _menu->menuFlags.set(old_flag);
            }
        } else logger::error("MenuHook: {}, menu not found", menuName);
    }

	logger::info("MenuHook: {}", a_message.menu);
    return _ProcessMessage(this, a_message);
}

void Hooks::Install(){
    MenuHook<RE::ContainerMenu>::InstallHook(RE::VTABLE_ContainerMenu[0]);
    MenuHook<RE::BarterMenu>::InstallHook(RE::VTABLE_BarterMenu[0]);
    MenuHook<RE::CraftingMenu>::InstallHook(RE::VTABLE_CraftingMenu[0]);
    MenuHook<RE::DialogueMenu>::InstallHook(RE::VTABLE_DialogueMenu[0]);
    MenuHook<RE::FavoritesMenu>::InstallHook(RE::VTABLE_FavoritesMenu[0]);
	MenuHook<RE::InventoryMenu>::InstallHook(RE::VTABLE_InventoryMenu[0]);
    MenuHook<RE::LockpickingMenu>::InstallHook(RE::VTABLE_LockpickingMenu[0]);
    MenuHook<RE::MagicMenu>::InstallHook(RE::VTABLE_MagicMenu[0]);
    MenuHook<RE::MapMenu>::InstallHook(RE::VTABLE_MapMenu[0]);

    SaveHook::InstallHook();

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(Hooks::trampoline_size);
    SaveDebugNotifHook::InstallHook();

    //MenuControlsHook::InstallHook();
};

void Hooks::SaveDebugNotifHook::thunk(const char* a1, uint64_t a2, char a3) {
    if ((_strcmpi(a1, "Autosaving...") == 0 || _strcmpi(a1, "Saving...") == 0) &&
        SaveSettings::block_autosaving_notif) {
        SaveSettings::block_autosaving_notif = false;
        return;
	}
    func(a1, a2, a3);
};

void Hooks::SaveDebugNotifHook::InstallHook(){
    auto& trampoline = SKSE::GetTrampoline();
    REL::Relocation<std::uintptr_t> originalFunc{RELOCATION_ID(50737, 51632)};
    func = trampoline.write_call<5>(originalFunc.address() + REL::Relocate(0xaf, 0xaf), thunk);
};

void Hooks::SaveHook::Accept(RE::Journal_SystemTab* a_this, RE::CallbackProcessor* a_cbReg)
{
	logger::info("SaveHook: Accept");
	originalFunction(a_this, a_cbReg);
}

void Hooks::SaveHook::Process(RE::FxDelegateHandler::CallbackProcessor* a_this, const RE::GString& a_methodName, CallbackFn* a_method)
{
	logger::info("SaveHook: {}", a_methodName.c_str());
	originalFunction2(a_this, a_methodName, a_method);
}

void Hooks::SaveHook::InstallHook()
{
    originalFunction = REL::Relocation<std::uintptr_t>(RE::Journal_SystemTab::VTABLE[0]).write_vfunc(0x1, Accept);
	originalFunction2 = REL::Relocation<std::uintptr_t>(RE::VTABLE_FxDelegateHandler__CallbackProcessor[0]).write_vfunc(0x1, Process);

}
