#pragma once

// https://github.dev/digital-apple/ad-mortem-permadeath/ <3
class Data
{
public:
    static auto GetSingleton() -> Data*;

    auto GetSaveFileDirectory() -> std::optional<std::filesystem::path>;

    static bool DeleteSaveFile(const RE::BSFixedString& fileName);

    Data& operator=(const Data&) = delete;
    Data& operator=(Data&&) = delete;
private:
    Data() = default;
    Data(const Data&) = delete;
    Data(Data&&) = delete;

    ~Data() = default;

    std::optional<std::filesystem::path> save_files;
};