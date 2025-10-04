#pragma once
#include <string>
#include <vector>
#include <random>
#include <thread>
#include <functional>
#include <mutex>
#include <Windows.h>
#include <dwmapi.h>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/// @brief Handles reading, writing, and repairing of the config file (`info.json`) 
///        for Moon Instance Controller. 
///        Stores references to main program state via raw pointers.
class MoonFileHandler
{
private:
    // Pointers to variables owned by the main program
    std::string* USER_NAME = nullptr;
    std::string* MOON_RBX_TEXT_TYPE = nullptr;
    std::string* MOON_RBX_COLOR_TYPE = nullptr;
    bool* PERFORMANCE_MODE = nullptr;
    bool* CPU_LIMITER = nullptr;
    bool* NEW_USER = nullptr;

    /// @brief Shows an error message in a MessageBox with the last Windows error code.
    void handleFileErrMessage(const std::string& msg)
    {
        DWORD err = GetLastError();
        std::string errCode = "[Error CODE " + std::to_string(err) + "]: " + msg;

        // Convert to wide string for MessageBoxW
        std::wstring formattedErrCode(errCode.begin(), errCode.end());
        MessageBoxW(nullptr, formattedErrCode.c_str(), L"Error", MB_ICONERROR);
    }

    /// @brief Repair or recreate the config file if missing or corrupted.
    void fixFilesAttempt(bool fileExists)
    {
        handleFileErrMessage("Problem accessing info.json... Attempting to repair.");

        try
        {
            if (fileExists)
                std::filesystem::remove("info.json");

            // Create default config
            json j;
            j["NEW_USER"] = false;
            j["CPU_LIMITOR"] = true;
            j["PEFORMANCE_MODE"] = false;
            j["USER_NAME"] = "Guest";
            j["MOON_RBX_TEXT_TYPE"] = "";
            j["MOON_RBX_COLOR_TYPE"] = "";

            std::ofstream outFile("info.json");
            outFile << j.dump(4);
            outFile.close();

            MessageBoxW(nullptr, L"Config repaired. Please reopen program.", L"Info", MB_ICONINFORMATION);
            exit(0);
        }
        catch (...)
        {
            handleFileErrMessage("Error interacting with info.json. Try running as Admin or reinstall.");
            exit(0);
        }
    }

public:
    /// @brief Link this handler to main program variables (references via pointers).
    void setJSONValues(bool& new_user,
        bool& performance_mode,
        bool& cpu_limiter,
        std::string& user_name,
        std::string& moon_rbx_text_type,
        std::string& moon_rbx_color_type)
    {
        NEW_USER = &new_user;
        PERFORMANCE_MODE = &performance_mode;
        CPU_LIMITER = &cpu_limiter;
        USER_NAME = &user_name;
        MOON_RBX_TEXT_TYPE = &moon_rbx_text_type;
        MOON_RBX_COLOR_TYPE = &moon_rbx_color_type;
    }

    /// @brief Ensure `info.json` exists and load values into linked variables.
    bool readRequiredFiles()
    {
        if (!std::filesystem::exists("info.json"))
        {
            try
            {
                // First run, create default config
                json j;
                j["NEW_USER"] = false;
                j["CPU_LIMITOR"] = true;
                j["PEFORMANCE_MODE"] = false;
                j["USER_NAME"] = "Guest";
                j["MOON_RBX_TEXT_TYPE"] = "";
                j["MOON_RBX_COLOR_TYPE"] = "";

                MessageBoxW(nullptr,
                    L"Welcome Guest, to V5 Moon Instance Controller!\nThe program will adjust a few things before starting.",
                    L"Greeting", MB_ICONINFORMATION);

                std::ofstream outFile("info.json");
                outFile << j.dump(4);
                outFile.close();
            }
            catch (...)
            {
                handleFileErrMessage("Error creating info.json. Run as Admin or reinstall.");
                exit(0);
            }
        }

        // Open file
        std::ifstream inFile("info.json");
        if (!inFile)
        {
            if (GetLastError() == ERROR_ACCESS_DENIED)
            {
                handleFileErrMessage("Access denied to info.json. Run as Admin.");
                exit(0);
            }

            if (std::filesystem::exists("info.json"))
                fixFilesAttempt(true);
            else
                fixFilesAttempt(false);

            return false;
        }

        try
        {
            json j;
            inFile >> j;
            inFile.close();

            // Validate all required fields
            if (!j.contains("NEW_USER") || !j["NEW_USER"].is_boolean()) fixFilesAttempt(false);
            if (!j.contains("CPU_LIMITOR") || !j["CPU_LIMITOR"].is_boolean()) fixFilesAttempt(false);
            if (!j.contains("PEFORMANCE_MODE") || !j["PEFORMANCE_MODE"].is_boolean()) fixFilesAttempt(false);
            if (!j.contains("USER_NAME") || !j["USER_NAME"].is_string()) fixFilesAttempt(false);
            if (!j.contains("MOON_RBX_COLOR_TYPE") || !j["MOON_RBX_COLOR_TYPE"].is_string()) fixFilesAttempt(false);
            if (!j.contains("MOON_RBX_TEXT_TYPE") || !j["MOON_RBX_TEXT_TYPE"].is_string()) fixFilesAttempt(false);

            // Assign values into linked variables
            *NEW_USER = j["NEW_USER"];
            *PERFORMANCE_MODE = j["PEFORMANCE_MODE"];
            *CPU_LIMITER = j["CPU_LIMITOR"];
            *USER_NAME = j["USER_NAME"];
            *MOON_RBX_TEXT_TYPE = j["MOON_RBX_TEXT_TYPE"];
            *MOON_RBX_COLOR_TYPE = j["MOON_RBX_COLOR_TYPE"];

            return true;
        }
        catch (...)
        {
            if (std::filesystem::exists("info.json"))
                fixFilesAttempt(true);
            else
                fixFilesAttempt(false);

            return false;
        }
    }

    /// @brief Save edited values into `info.json`.
    void editValues(const std::string& user_name,
        const std::string& moonrbx_text,
        const std::string& moonrbx_color,
        bool cpu_limiter,
        bool performance_mode)
    {
        // Update referenced variables
        *USER_NAME = user_name;
        *CPU_LIMITER = cpu_limiter;
        *PERFORMANCE_MODE = performance_mode;
        *MOON_RBX_TEXT_TYPE = moonrbx_text;
        *MOON_RBX_COLOR_TYPE = moonrbx_color;

        try
        {
            json j;
            j["NEW_USER"] = false;
            j["CPU_LIMITOR"] = *CPU_LIMITER;
            j["PEFORMANCE_MODE"] = *PERFORMANCE_MODE;
            j["USER_NAME"] = *USER_NAME;
            j["MOON_RBX_COLOR_TYPE"] = *MOON_RBX_COLOR_TYPE;
            j["MOON_RBX_TEXT_TYPE"] = *MOON_RBX_TEXT_TYPE;

            std::ofstream outFile("info.json");
            outFile << j.dump(4);
            outFile.close();
        }
        catch (...)
        {
            handleFileErrMessage("Error writing to info.json. Run as Admin or reinstall.");
            exit(0);
        }
    }
};
