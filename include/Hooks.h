#pragma once
#include "Settings.h"

namespace Hooks {
    constexpr uint8_t n_hooks = 1;
    constexpr size_t trampoline_size = n_hooks * 14;

    void Install();

    template <typename MenuType>
    class MenuHook : public MenuType {
        using ProcessMessage_t = decltype(&MenuType::ProcessMessage);
        static inline REL::Relocation<ProcessMessage_t> _ProcessMessage;
        RE::UI_MESSAGE_RESULTS ProcessMessage_Hook(RE::UIMessage& a_message);
    public:
        static void InstallHook(const REL::VariantID& varID);
    };

    struct SaveDebugNotifHook {
        static void thunk(const char* a1, uint64_t a2, char a3);
        static inline REL::Relocation<decltype(thunk)> func;
		static void InstallHook();
    };

    struct SaveHook {
        using CallbackFn = void(const RE::FxDelegateArgs& a_params);
        static inline REL::Relocation<void(RE::Journal_SystemTab*,RE::CallbackProcessor*)> originalFunction;
        static inline REL::Relocation<void(RE::FxDelegateHandler::CallbackProcessor*,const RE::GString&, CallbackFn*)> originalFunction2;
        
		static void Accept(RE::Journal_SystemTab* a_this, RE::CallbackProcessor* a_cbReg);
        //VTABLE_FxDelegateHandler__CallbackProcessor
		static void Process(RE::FxDelegateHandler::CallbackProcessor* a_this, const RE::GString& a_methodName, CallbackFn* a_method);

        static void InstallHook();
    };

};