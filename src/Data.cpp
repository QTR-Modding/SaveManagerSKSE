#include "Data.h"
#include "REX/W32/OLE32.h"
#include "REX/W32/SHELL32.h"


auto Data::GetSingleton() -> Data*
{
    static Data singleton;
    return &singleton;
}

auto Data::GetSaveFileDirectory() -> std::optional<std::filesystem::path>
{
    if (!save_files) {
        const auto ini_settings = RE::INISettingCollection::GetSingleton();

        if (!ini_settings) {
            return std::nullopt;
        }

        wchar_t* buffer = nullptr;
        const auto result = REX::W32::SHGetKnownFolderPath(REX::W32::FOLDERID_Documents, REX::W32::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));

        std::unique_ptr<wchar_t[], decltype(&REX::W32::CoTaskMemFree)> path_ptr(buffer, REX::W32::CoTaskMemFree);

        if (!path_ptr || result != 0) {
            return std::nullopt;
        }

        std::filesystem::path path = path_ptr.get();

        path /= "My Games"sv;
        path /= std::filesystem::exists("steam_api64.dll") ? "Skyrim Special Edition" : "Skyrim Special Edition GOG";
        path /= ini_settings->GetSetting("sLocalSavePath:General")->GetString();

        save_files = path;

        return path;
    }

    return save_files;
}

bool Data::DeleteSaveFile(const RE::BSFixedString& fileName)
{
    if (fileName.empty()) return false;
	const auto save_dir = Data::GetSingleton()->GetSaveFileDirectory();
    if (!save_dir) { logger::critical("Failed to obtain save files path!"); return false;}

    try {
        if (std::filesystem::exists(*save_dir)) {
            for (const auto& entry : std::filesystem::directory_iterator(*save_dir)) {
                if (entry.path().stem() == fileName.data()) {
                    logger::info("Deleting file: '{}'", entry.path().filename().string());
                    std::filesystem::remove(entry);
					return true;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& error) {
		logger::critical("Addresses::DeathHandler :: An error occurred while deleting save files: {}", error.what());
		return false;
    }

	return true;
}
