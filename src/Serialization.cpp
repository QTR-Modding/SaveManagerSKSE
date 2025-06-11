#include "Serialization.h"
#include "Settings.h"

void SaveCallback(SKSE::SerializationInterface*) { if (SaveSettings::auto_save_to_json) SaveSettings::SaveJSON(); }

void InitializeSerialization() {
    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(kDataKey);
    serialization->SetSaveCallback(SaveCallback);
    // serialization->SetLoadCallback(LoadCallback);
    SKSE::log::trace("Cosave serialization initialized.");
}