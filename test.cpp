// GUI credits to unknown.e, Code Logic Written by Noah

// imports
#include <random>
#include <queue>
#include <tuple>
#include <mutex>
#include <format>
#include <Windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdio>
#include <atomic>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string>
#include <set>
#include <tlhelp32.h>
#include <wchar.h>
#include <filesystem>
//#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <uxtheme.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <wingdi.h>
#include <fwpmu.h>
#include <shlwapi.h>

using json = nlohmann::json;

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "fwpuclnt.lib")
#pragma comment(lib, "Shlwapi.lib")

// Public UI IDs
#define ID_CREDITS 0
#define ID_WALK 1
#define ID_JUMP 2
#define ID_SLOW_WALK 3
#define ID_SEND_MESSAGE 4
#define ID_SPAM_MESSAGE 5
#define ID_DANCE 6
#define ID_DANCE2 7
#define ID_DANCE3 8
#define ID_DANCELOOP 9
#define ID_UNDANCE 10
#define ID_WAVE 11
#define ID_POINT 12
#define ID_TURN_AROUND 13
#define ID_WALK_BACKWARD 14
#define ID_WALK_RIGHT 15
#define ID_WALK_LEFT 16
#define ID_FREEZE 17
#define ID_STADIUM 19
#define ID_CHEER 20
#define ID_LAUGH 21
#define ID_SHRUG 22
#define ID_SETTINGS 23
#define ID_CPU_USAGE 24
#define ID_PEFORMANCE 25
#define ID_NAME 26
#define ID_CONFIRM 27
#define ID_PM_MESSAGE 28
#define ID_HACKING 29
#define ID_UPDATE 30
#define ID_LANGUAGE_TROLL 31
#define ID_EVACUATE 32
#define ID_RESET 33
#define ID_SPIN 34
#define ID_LAG 36
#define WM_UPDATE_SIDEBAR (WM_USER + 1)
#define VM_HANDLE_MODE (WM_UPDATE_SIDEBAR + 1)

// Other Public Variables
std::atomic<bool> exitFlag(false);
std::atomic<bool> instanceExitFlag(false);
std::mutex userThreadMutex;
std::random_device rd;
std::mt19937 gen(rd());

// Global UI Elements
HWND g_MainWindow = NULL;
HWND hChatTextbox = NULL;
HWND hPmUserTextbox = NULL;
HWND hCpuUsage = NULL;
HWND hRamUsage = NULL;
HWND hPeformance = NULL;
HWND hName = NULL;
HWND masterClient = NULL;
HINSTANCE g_hInstance = NULL;

// Other Global Variables
std::vector<HWND> listOfConnectedClients = {};
std::vector<HWND> listOfAllUI = {};

struct userProcessDetails
{
    bool isRunning;
    bool managesMoonTimes;
    DOUBLE cpuCap;
    DOUBLE cpuUsage;
    DWORD pid;
    HWND hwnd;
    HANDLE hProcess;
    HANDLE hJob;
};
std::vector<std::shared_ptr<userProcessDetails>> userProcessDetailsList;


// Client Information
std::string mode = "Normal";
std::string CLIENT_VERSION = "V5";
std::string USER_NAME;

// Default values
bool CPU_LIMITOR = false; 
bool NEW_USER = false;
bool PEFORMANCE_MODE = false;
bool lagEnabled = false;
bool warningSent = false;
bool operationRunning = false;
int ENABLE_SLASH_PAUSE = 0;
int WAIT_TIME = 30;

// UI Color
HBRUSH hBrushDarkGrey = NULL;
HBRUSH hButtonBrush = CreateSolidBrush(RGB(60, 60, 60));
HBRUSH hButtonHoverBrush = CreateSolidBrush(RGB(90, 150, 200));
HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(40, 40, 40));
HBRUSH hLagEnabledBrush = CreateSolidBrush(RGB(0, 170, 60));
HBRUSH hLagDisabledBrush = CreateSolidBrush(RGB(220, 40, 40));

// Font used for UI
HFONT hFont = CreateFontW(
    27, 0, 0, 0, FW_LIGHT,
    false, false, false,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    CLEARTYPE_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE,
    L"Segoe UI"
);
HFONT hFontGreetings = CreateFont(35, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

// Handling error messages
void handleErrorMessage(const std::string& msg)
{
    DWORD err = GetLastError();
    std::string errCode = "[Error CODE " + std::to_string(err) + "]: " + msg;
    std::wstring formattedErrCode(errCode.begin(), errCode.end());
    MessageBox(NULL, formattedErrCode.c_str(), L"Error", MB_ICONERROR);
}

// Attempt to fix handle
HANDLE attemptFixHandle(HANDLE& hProcess, DWORD& pid, std::string type)
{
     int trialAttempts = 0;
     bool success = false;

     while (trialAttempts < 5)
     {
         std::unique_lock<std::mutex> lock(userThreadMutex);
         if (type == "Process")
         {
             hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SET_INFORMATION | PROCESS_SET_QUOTA, FALSE, pid);
         }
         else
         {
             hProcess = CreateJobObject(nullptr, nullptr); 
         }

         if (hProcess != NULL)
         {
             success = true;
             break;
         }
         
         trialAttempts++;
         lock.unlock();

         std::this_thread::sleep_for(std::chrono::milliseconds(50));
     }

    if (!success)
    {
        handleErrorMessage("There has been a fatal error, we have to quit, sorry! MOON_FATAL_ERROR: Failed to open Roblox Process, trying running as administrator, or re-installing.");
        exit(0);
    }

    return hProcess;
}


// General input
void sendKey(WORD vk)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = vk;
    down.ki.wScan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &up, sizeof(INPUT));
    SendInput(1, &down, sizeof(INPUT));

    if (vk == VK_OEM_2 && ENABLE_SLASH_PAUSE != 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME - ENABLE_SLASH_PAUSE));
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    }
}


// Special key functions
void pressEnter() { sendKey(VK_RETURN); }
void pressSpace() { sendKey(VK_SPACE); }
void pressEscape() { sendKey(VK_ESCAPE); }
void pressDelete() { sendKey(VK_BACK); }
void pressSlash() { sendKey(VK_OEM_2); }

// Letter keys
void pressW() { sendKey('W'); }
void pressA() { sendKey('A'); }
void pressS() { sendKey('S'); }
void pressD() { sendKey('D'); }
void pressR() { sendKey('R'); }

// CtrlV
void pressCtrlV(bool fixCopyPaste = false)
{
    INPUT inputs[4] = {};

    // Press CTRL
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 0;
    inputs[0].ki.wScan = MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 0;
    inputs[1].ki.wScan = MapVirtualKey('V', MAPVK_VK_TO_VSC);
    inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE;

    inputs[2] = inputs[1];
    inputs[2].ki.dwFlags |= KEYEVENTF_KEYUP;

    inputs[3] = inputs[0];
    inputs[3].ki.dwFlags |= KEYEVENTF_KEYUP;

    if (fixCopyPaste)
    {
        INPUT release[2] = {};

        // Release CTRL + V for safety
        release[0] = inputs[3];
        release[0].ki.dwFlags |= KEYEVENTF_KEYUP;

        release[1] = inputs[2];
        release[1].ki.dwFlags |= KEYEVENTF_KEYUP;

        SendInput(ARRAYSIZE(release), release, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    }
    else {
        SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME + 5));
    }

}


/*
// Buffer Read
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Buffer Download
size_t WriteFileCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    if (stream == nullptr) return 0; // Null check
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

// Receive Response
std::string receiveResp(CURL*& curl, const char* url, FILE* filePtr = nullptr)
{
    std::string readBuffer;

    // Settings options // headers
    curl_easy_setopt(curl, CURLOPT_URL, url);

    if (filePtr != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, filePtr);
    }
    else
    {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    }

    // Performing CURL Request and getting response
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        // Error occurred, return empty string
        return "";
    }
    else
    {
        return readBuffer;
    }
}

void installNewUpdate(CURL*& curl, const std::string& version)
{
    // Opening up new File
    std::string fileName = "Moon Client Controller V" + version + ".exe";
    std::wstring versionFormatted(version.begin(), version.end());
    std::wstring updateMessageFormatted = std::format(L"Updated to Moon Client Controller V{}, please delete this old file, and open the new updated one.", versionFormatted);

    // Attempt to create new file to stream the data into
    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, fileName.c_str(), "wb");
    if (err != 0 || !fp) {
        // handle error, e.g. call handleErrorMessage
        handleErrorMessage("Failed to open file for writing.");
        return;
    }

    // Sending Request && Installing
    long httpCode = 0;
    receiveResp(curl, "https://raw.githubusercontent.com/thanknoah/Moon-Instance-Controller/main/Moon%20Client%20Controller.exe", fp);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    // Check file size before closing
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);

    fclose(fp);

    // Validate HTTP response and file size
    if (httpCode != 200 || fileSize == 0)
    {
        // Delete the incomplete or empty file
        remove(fileName.c_str());

        curl_easy_cleanup(curl);
        curl_global_cleanup();
        handleErrorMessage("While trying to download latest update. Website might be down, or permissions are insufficient. Please try again later or reinstall.");
        return;
    }
    else
    {
        MessageBox(NULL, updateMessageFormatted.c_str(), L"Update", MB_ICONINFORMATION);
        exit(0);
    }
}

void updateChecker()
{
    // Initializing Curl
    CURLcode global_init = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (global_init != CURLE_OK)
    {
        handleErrorMessage(curl_easy_strerror(global_init));
        return;
    }

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        curl_global_cleanup();
        handleErrorMessage("While trying to initialize curl, please reinstall application.");
        return;
    }

    bool installing = false;

    // Check version
    std::string resp = receiveResp(curl, "https://raw.githubusercontent.com/thanknoah/Moon-Instance-Controller/main/version.txt", nullptr);

    if (!resp.empty())
    {
        // Check if CLIENT_VERSION is anywhere in the response
        if (resp.find(CLIENT_VERSION) != std::string::npos)
        {
            installNewUpdate(curl, resp);
        }
    }
    else
    {
        handleErrorMessage("While trying to receive response from curl version.txt, website is down (wait few hours) or reinstall application.");
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}
*/

// Total amount of Roblox Instances Open
std::vector<HWND> getRobloxWindows()
{
    // Searches for Roblox Instances
    std::vector<HWND> instances;
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            wchar_t title[256];
            GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));

            if (wcsstr(title, L"Roblox"))
            {
                ((std::vector<HWND>*)lParam)->push_back(hwnd);
            }


            return true;
        }, (LPARAM)&instances);

    // Cleans up table
    for (auto it = instances.begin(); it != instances.end(); )
    {
        wchar_t title[256];
        const wchar_t* robloxName = L"Roblox";
        GetWindowText(*it, title, sizeof(title) / sizeof(wchar_t));

        if (wcscmp(title, robloxName) != 0) { it = instances.erase(it); }
        if (wcscmp(title, robloxName) == 0) { ++it; }
    }
    return instances;
}

// Total Amount Of Moon Roblox Clients Opened
std::vector<HWND> getMoonWindows()
{
    // Searches for Roblox Instances
    std::vector<HWND> instances;
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            wchar_t title[256];
            GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));

            if (wcsstr(title, L"MoonRBX V1"))
            {
                ((std::vector<HWND>*)lParam)->push_back(hwnd);
            }


            return true;
        }, (LPARAM)&instances);

    // Cleans up table
    for (auto it = instances.begin(); it != instances.end(); )
    {
        wchar_t title[256];
        const wchar_t* robloxName = L"MoonRBX V1";
        GetWindowText(*it, title, sizeof(title) / sizeof(wchar_t));

        if (wcscmp(title, robloxName) != 0) { it = instances.erase(it); }
        if (wcscmp(title, robloxName) == 0) { ++it; }
    }
    return instances;
}

// Admin permissions
void getAdminPerms(bool& successful)
{
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &adminGroup))
    {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    if (!isAdmin)
    {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        INT_PTR res = (INT_PTR)ShellExecuteW(
            NULL,
            L"runas",
            path,
            NULL, // WILL COME BACK TO LATER
            NULL,
            SW_SHOWNORMAL
        );

        if (res > 32)
            ExitProcess(0);
        else
            successful = false;
    }
}

// Get roblox path location
std::wstring getRobloxFileLocation()
{
    DWORD pid = 0;
    std::vector<HWND> listOfClients = getMoonWindows();
    GetWindowThreadProcessId(listOfClients[0], &pid);

    HANDLE robloxClient = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (robloxClient != NULL)
    {
        TCHAR path[MAX_PATH];
        DWORD sizeOfPath = MAX_PATH;

        if (QueryFullProcessImageName(robloxClient, 0, path, &sizeOfPath))
        {
            CloseHandle(robloxClient);
            return path;
        }

        CloseHandle(robloxClient);
        return L"NULL";
    }
    else
    {
        return L"NULL";
    }
}


// Lag switch
void enableAndDisableLagSwitch(bool enabled)
{
    std::wstring path = getRobloxFileLocation();
    if (path == L"NULL")
    {
        handleErrorMessage("Failed to retrieve Roblox path.");
        return;
    }

    // Commands
    std::wstring deleteCmd =
        L"cmd.exe /c netsh advfirewall firewall delete rule name=\"Block RobloxPlayerBeta\"";

    std::wstring createCmd =
        L"cmd.exe /c netsh advfirewall firewall add rule name=\"Block RobloxPlayerBeta\" "
        L"dir=out action=block program=\"" + path + L"\" enable=yes";

    std::wstring disableCmd =
        L"cmd.exe /c netsh advfirewall firewall set rule name=\"Block RobloxPlayerBeta\" new enable=no";

    auto runCommand = [](const std::wstring& cmd)
        {
            STARTUPINFOW si{};
            PROCESS_INFORMATION pi{};
            si.cb = sizeof(si);

            BOOL success = CreateProcessW(
                nullptr,
                const_cast<wchar_t*>(cmd.c_str()),
                nullptr,
                nullptr,
                FALSE,
                CREATE_NO_WINDOW,
                nullptr,
                nullptr,
                &si,
                &pi
            );

            if (success)
            {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
            else
            {
                handleErrorMessage("Failed to execute command.");
            }
        };

    if (enabled)
    {
        runCommand(deleteCmd);
        runCommand(createCmd);
    }
    else
    {
        runCommand(disableCmd);
    }
}


// Fix self attempt
void fixFilesAttempt(bool fileExists)
{
    handleErrorMessage("Happened while trying to access info.json.. Will attempt to fix..");
    try 
    {
        if (fileExists)
           std::filesystem::remove("info.json");

        json j;
        j["NEW_USER"] = false;
        j["CPU_LIMITOR"] = true;
        j["PEFORMANCE_MODE"] = false;
        j["USER_NAME"] = "Guest";

        std::ofstream outFile("info.json");
        outFile << j.dump(4);
        outFile.close();
        MessageBox(NULL, L"Fixed file! Please reopen program.", L"Error", MB_ICONINFORMATION);
        exit(0);

    }
    catch (...)
    {
        handleErrorMessage("Happened while trying to interacting with info.json, please run with admin permissions, or reinstall program.");
        exit(0);
    }
}

// Check if required files are here
bool readRequiredFiles()
{  
    if (!std::filesystem::exists("info.json"))
    {
        try 
        {
            json j;
            j["NEW_USER"] = false;
            j["CPU_LIMITOR"] = true;
            j["PEFORMANCE_MODE"] = false;
            j["USER_NAME"] = "Guest";
            MessageBox(NULL, L"It appears your new. Welcome Guest, to V5 Moon Instance Controller! The program will now adjust a few things, then you can get right into it.", L"Greeting", MB_ICONINFORMATION);

            std::ofstream outFile("info.json");
            outFile << j.dump(4);
            outFile.close();
        }
        catch (...)
        {
            handleErrorMessage("Happened while trying to interacting with info.json, please run with admin permissions, or reinstall program.");
            exit(0);
        }
    }

    std::ifstream inFile("info.json");
    if (!inFile)
    {
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
            handleErrorMessage("Happened while trying to interacting with info.json, please run with admin permissions, or reinstall program.");
            exit(0);
        }

        if (std::filesystem::exists("info.json"))
        {
            fixFilesAttempt(true);
            return false;
        }
        else
        {
            fixFilesAttempt(false);
            return false;
        }
    }

    try {
        json j;
        inFile >> j;
        inFile.close();

        if (!j.contains("NEW_USER") || !j["NEW_USER"].is_boolean()) fixFilesAttempt(false);
        if (!j.contains("CPU_LIMITOR") || !j["CPU_LIMITOR"].is_boolean()) fixFilesAttempt(false);
        if (!j.contains("PEFORMANCE_MODE") || !j["PEFORMANCE_MODE"].is_boolean()) fixFilesAttempt(false);
        if (!j.contains("USER_NAME") || !j["USER_NAME"].is_string()) fixFilesAttempt(false);

        NEW_USER = j["NEW_USER"];
        PEFORMANCE_MODE = j["PEFORMANCE_MODE"];
        CPU_LIMITOR = j["CPU_LIMITOR"];
        USER_NAME = j["USER_NAME"];

        return true;
    }
    catch (...)
    {
        if (std::filesystem::exists("info.json"))
        {
            fixFilesAttempt(true);
            return false;
        }
        else
        {
            fixFilesAttempt(false);
            return false;
        }
    }
}

// Edit files
void editValues(std::string& user_name, bool cpu_limitor, bool peformance_mode)
{
    USER_NAME = user_name;
    CPU_LIMITOR = cpu_limitor;
    PEFORMANCE_MODE = peformance_mode;
    try
    {
        json j;
        j["NEW_USER"] = false;
        j["CPU_LIMITOR"] = CPU_LIMITOR;
        j["PEFORMANCE_MODE"] = PEFORMANCE_MODE;
        j["USER_NAME"] = USER_NAME;

        std::ofstream outFile("info.json");
        outFile << j.dump(4);
        outFile.close();
    }
    catch (...)
    {
        handleErrorMessage("Happened while trying to interacting with info.json, please run with admin permissions, or reinstall program.");
        exit(0);
    }
}


// Detect if exit is clicked (ctrl + e)
void exitDetection()
{
    bool wasPressed = false;

    while (true)
    {
        bool detectEPressed = (GetAsyncKeyState('E') & 0x8000);
        bool detectCtrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
        bool combination = detectEPressed && detectCtrlPressed;
        int msToWait = (userProcessDetailsList.size() * 45) + 50;

        if (combination && !wasPressed)
        {
            exitFlag.store(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(msToWait));
            exitFlag.store(false);
            wasPressed = true;
        }
        else if (!combination)
        {
            wasPressed = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Run Every 1 milliseconds (detector)
    }
}

// Gets PIDs
std::vector<DWORD> getPids(std::vector<HWND>& customizableInstances)
{
    std::vector<DWORD> pids;
    DWORD pid = 0;

    for (HWND instance : customizableInstances)
    {
        GetWindowThreadProcessId(instance, &pid);
        if (pid != 0)
            pids.push_back(pid);
    }

    return pids;
}


// Smart RAM Allocater
void getCpuUsageInfoForAllClients(std::shared_ptr<userProcessDetails> newDetails)
{
    // Main Loop
    while (newDetails->isRunning)
    {   
        try {
            // Process info
            FILETIME fileCreation, fileExit, fileKernel, fileUser;
            ULARGE_INTEGER creationTime1, exitTime1, kernelTime1, userTime1;

            GetProcessTimes(newDetails->hProcess, &fileCreation, &fileExit, &fileKernel, &fileUser);
            memcpy(&creationTime1, &fileCreation, sizeof(FILETIME));
            memcpy(&exitTime1, &fileExit, sizeof(FILETIME));
            memcpy(&kernelTime1, &fileKernel, sizeof(FILETIME));
            memcpy(&userTime1, &fileUser, sizeof(FILETIME));

            // System info
            FILETIME sysIdle, sysKernel, sysUser;
            ULARGE_INTEGER idleSys1, kernelSys1, userSys1;

            GetSystemTimes(&sysIdle, &sysKernel, &sysUser);
            memcpy(&idleSys1, &sysIdle, sizeof(FILETIME));
            memcpy(&kernelSys1, &sysKernel, sizeof(FILETIME));
            memcpy(&userSys1, &sysUser, sizeof(FILETIME));

            // =========================
            // Wait a bit before 2nd snapshot
            // =========================

            std::this_thread::sleep_for(std::chrono::milliseconds(350));


            // Process info
            FILETIME fileCreation2, fileExit2, fileKernel2, fileUser2;
            ULARGE_INTEGER creationTime2, exitTime2, kernelTime2, userTime2;

            GetProcessTimes(newDetails->hProcess, &fileCreation2, &fileExit2, &fileKernel2, &fileUser2);
            memcpy(&creationTime2, &fileCreation2, sizeof(FILETIME));
            memcpy(&exitTime2, &fileExit2, sizeof(FILETIME));
            memcpy(&kernelTime2, &fileKernel2, sizeof(FILETIME));
            memcpy(&userTime2, &fileUser2, sizeof(FILETIME));

            // System info
            FILETIME sysIdle2, sysKernel2, sysUser2;
            ULARGE_INTEGER idleSys2, kernelSys2, userSys2;

            GetSystemTimes(&sysIdle2, &sysKernel2, &sysUser2);
            memcpy(&idleSys2, &sysIdle2, sizeof(FILETIME));
            memcpy(&kernelSys2, &sysKernel2, sizeof(FILETIME));
            memcpy(&userSys2, &sysUser2, sizeof(FILETIME));

            // Process delta
            ULONGLONG procKernelDiff = kernelTime2.QuadPart - kernelTime1.QuadPart;
            ULONGLONG procUserDiff = userTime2.QuadPart - userTime1.QuadPart;
            ULONGLONG procTotalDiff = procKernelDiff + procUserDiff;

            // System delta
            ULONGLONG sysKernelDiff = kernelSys2.QuadPart - kernelSys1.QuadPart;
            ULONGLONG sysUserDiff = userSys2.QuadPart - userSys1.QuadPart;
            ULONGLONG sysTotalDiff = sysKernelDiff + sysUserDiff;
            double cpuPercent = 0;

            // Total cpu usage
            if (newDetails->managesMoonTimes)
            {
                ULONGLONG sysIdleDiff = idleSys2.QuadPart - idleSys1.QuadPart;
                ULONGLONG sysKernelDiff = kernelSys2.QuadPart - kernelSys1.QuadPart;
                ULONGLONG sysUserDiff = userSys2.QuadPart - userSys1.QuadPart;
                ULONGLONG sysBusyDiff = (sysKernelDiff + sysUserDiff) - sysIdleDiff;
                ULONGLONG sysTotalDiff = sysKernelDiff + sysUserDiff;

                double totalCpuUsage = 0.0;
                if (sysTotalDiff > 0)
                    totalCpuUsage = (double)sysBusyDiff / (double)sysTotalDiff * 100.0;

                if ((int)totalCpuUsage > 79)
                    ENABLE_SLASH_PAUSE = 15;
                else if ((int)totalCpuUsage > 67)
                    ENABLE_SLASH_PAUSE = 15;
                else if ((int)totalCpuUsage > 50)
                    ENABLE_SLASH_PAUSE = 6;
                else
                    ENABLE_SLASH_PAUSE = 4;
                
                if ((int)totalCpuUsage < 75)
                {
                    if ((int)(totalCpuUsage * 0.48) < 24) 
                    { 
                        WAIT_TIME = 24; 
                    }
                    else 
                    { 
                        WAIT_TIME = (int)(totalCpuUsage * 0.48); 
                    }
                }
                else if ((int)totalCpuUsage > 75 && (int)totalCpuUsage < 79)
                {
                    WAIT_TIME = (int)(totalCpuUsage * 0.52);
                }
                else if ((int)totalCpuUsage > 82 && (int)totalCpuUsage < 90)
                {
                    WAIT_TIME = (int)(totalCpuUsage * 0.57);
                }
                else if ((int)totalCpuUsage > 90)
                {
                    WAIT_TIME = (int)(totalCpuUsage * 0.65);
                }
            }


            // Calculations
            if (sysTotalDiff > 0)
            {
                cpuPercent = double(procTotalDiff) / (double)sysTotalDiff * 100.0;
            }
            else
            {
                continue;
            }

            // Assigning
            newDetails->cpuUsage = cpuPercent;

            /*
            if (cpuUsageCollections.size() < 5)
            {
                cpuUsageCollections.push_back(cpuPercent);
            }
            if (cpuUsageCollections.size() == 5)
            {
                if (3.0 > cpuPercent)
                {
                    newDetails->cpuCap = 3.0;
                }
                else
                {
                    newDetails->cpuCap = *std::max_element(cpuUsageCollections.begin(), cpuUsageCollections.end());
                }
                cpuUsageCollections.clear();
            }
            */        
        }
        catch (...)
        {
            handleErrorMessage("Failed to measure CPU time");
            continue;
        }
    }
}


// New userThread
void addNewUserThread(const std::map<HWND, DWORD>& formattedAddedClients)
{
    for (auto it = formattedAddedClients.begin(); it != formattedAddedClients.end(); ++it)
    {
        HWND hwnd = it->first;
        DWORD pid = it->second;
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, true, pid);
        if (hProcess == NULL)
        {
            attemptFixHandle(hProcess, pid, "Process");
        }

        HANDLE hJob = CreateJobObject(nullptr, nullptr);
        if (hJob == NULL)
        {
            attemptFixHandle(hJob, pid, "Job");
        }

        std::shared_ptr<userProcessDetails> newDetails = std::make_shared<userProcessDetails>();
        newDetails->cpuUsage = 0.0;
        newDetails->cpuCap = 0.0;
        newDetails->isRunning = true;
        newDetails->pid = pid;
        newDetails->hwnd = hwnd;
        newDetails->hProcess = hProcess;
        newDetails->hJob = hJob;

        if (userProcessDetailsList.size() == 0)
        {
            newDetails->managesMoonTimes = true;
        }

        std::unique_lock<std::mutex> lock(userThreadMutex);
        userProcessDetailsList.emplace_back(newDetails);
        std::thread(getCpuUsageInfoForAllClients, newDetails).detach();

        lock.unlock();
    }
}

void applyJobsToUserThreads(std::vector<std::tuple<HWND, HANDLE, HANDLE>>& userInformation, bool derating)
{
    for (const auto& user : userInformation)
    {
        HANDLE hJob = std::get<2>(user);
        HANDLE hProcess = std::get<1>(user);

        JOBOBJECT_CPU_RATE_CONTROL_INFORMATION info = {}; 
        info.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP; 
        info.CpuRate = (derating == true) ? 400 : 10000; 
        
        if (!SetInformationJobObject(hJob, JobObjectCpuRateControlInformation, &info, sizeof(info))) { 
            handleErrorMessage("Error assigning CPU info to Job."); 
        }

        if (!AssignProcessToJobObject(hJob, hProcess)) {
            handleErrorMessage("Error assigning CPU limits to job");
        }
    }
}

// Retrieve all info
std::vector<std::tuple<HWND, HANDLE, HANDLE>> getAllUserThreadInfo()
{
    std::vector<std::tuple<HWND, HANDLE, HANDLE>> userInformation;
    std::unique_lock<std::mutex> lock(userThreadMutex);
    for (auto it = userProcessDetailsList.begin(); it != userProcessDetailsList.end(); ++it)
    {
        userInformation.push_back({ (*it)->hwnd, (*it)->hProcess, (*it)->hJob });
    }

    lock.unlock();
    return userInformation;
}

// Search userThread
bool searchUserThread(DWORD& pidSearch, double& var, std::string type)
{
    std::unique_lock<std::mutex> lock(userThreadMutex);
    for (auto it = userProcessDetailsList.begin(); it != userProcessDetailsList.end(); ++it)
    {
        if ((*it)->pid == pidSearch)
        {
            if (type == "cpuUsage")
            {
                var = (*it)->cpuUsage;
                if ((*it)->cpuUsage == 0.0) return false;
                return true;
            } 
            else if (type == "cpuCap")
            {
                var = (*it)->cpuCap;
                if ((*it)->cpuCap == 0.0) return false;
                return true;
            }   
        }
    }
    lock.unlock();
    return false;
}

// Alert
void notifyRemovalOfThread()
{
    instanceExitFlag.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    instanceExitFlag.store(false);
}

// Remove userThread
void removeUserThread(const std::vector<DWORD>& pids)
{
    std::thread(notifyRemovalOfThread).detach();
    for (DWORD pid : pids)
    {
        std::unique_lock<std::mutex> lock(userThreadMutex);
        for (auto it = userProcessDetailsList.begin(); it != userProcessDetailsList.end();)
        {
            if ((*it)->pid == pid)
            {
                (*it)->isRunning = false;
                it = userProcessDetailsList.erase(it);
            }
            else
            {
                ++it;
            }
        }
        lock.unlock();
    }
}


// Limit RAM && CPU Usage
void optimisePeformance()
{
    // Jobs
    //std::vector<HANDLE> jobs;

    while (true)
    {
        // Variables
        std::vector<HWND> listOfClients = getMoonWindows();
        std::vector<DWORD> pids = getPids(listOfClients);
        size_t pidSize = pids.size();

        // Memory info
        DWORDLONG availPhys = 0;
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        int count = 0;

        // Getting Available Memory
        if (GlobalMemoryStatusEx(&memInfo))
            availPhys = memInfo.ullAvailPhys * 0.8;


        // Skip
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        if (listOfClients.size() == 0) continue;

        /*
        // Deleting Jobs
        for (HANDLE job : jobs)
        {
            CloseHandle(job);
            jobs.erase(std::find(jobs.begin(), jobs.end(), job));
        }

        */


        // Applying limits
        for (DWORD pid : pids)
        {
            // Opening PID && Error Handling
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SET_INFORMATION | PROCESS_SET_QUOTA | PROCESS_TERMINATE, false, pid);
            if (hProcess == NULL)
            {
                if (!instanceExitFlag.load())
                    handleErrorMessage("Happened while trying to open Roblox Process ID, this error is non-critical, however if it persists, please run with admin permissions or disable CPU Limiter");
                continue;
            }
            /*
            // Creating Job object && Error Handling
            HANDLE hJob = CreateJobObject(NULL, NULL);
            if (hJob == NULL)
            {
                handleErrorMessage("Happened while trying to open Roblox Process ID, this error is non-critical, however if it persists, please run with admin permissions or disable CPU Limiter");
                CloseHandle(hProcess);
                continue;
            }

            JOBOBJECT_CPU_RATE_CONTROL_INFORMATION info = { 0 };
            info.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_MIN_MAX_RATE;
            */
            

            // Values
            size_t lowerRamLimit = 10ULL * 1024 * 1024; // Default values
            size_t upperRamLimit = 290ULL * 1024 * 1024; // Default values
            //info.MaxRate = 13 * 100;
            //info.MinRate = 1 * 100;

            // Values got from smart allocator
            double cpuUsage;

            // Upper Ram Limit Formula
            if (searchUserThread(pid, cpuUsage, "cpuUsage") && availPhys != 0)
            {
                // Limits 
                const size_t MIN_RAM = 150ULL * 1024 * 1024; // 150 MB
                const size_t MAX_RAM = 600ULL * 1024 * 1024; // 600 MB

                // Weight
                size_t baseAllocation = availPhys / pidSize;  // Equal share of physical RAM
                size_t adjustedAllocation = static_cast<size_t>(baseAllocation * ((100-cpuUsage)/100));

                // Clamp
                if (adjustedAllocation < MIN_RAM) adjustedAllocation = MIN_RAM;
                if (adjustedAllocation > MAX_RAM) adjustedAllocation = MAX_RAM;
                upperRamLimit = adjustedAllocation;
            }
            
            /*
            if (searchUserThread(pid, cpuCap, "cpuCap"))
            {
                double rounded = round(cpuCap);
                info.MaxRate = rounded * 100;
            }
            */

            HANDLE hJob = CreateJobObject(nullptr, nullptr);
            if (hJob == NULL)
            {
                handleErrorMessage("Failed to create job.");
            }


            // Setting RAM
            if (!SetProcessWorkingSetSizeEx(hProcess, lowerRamLimit, upperRamLimit, 0) && !instanceExitFlag.load())
            {
                if (!instanceExitFlag.load())
                    handleErrorMessage("Happened while trying to set RAM Usage Limit, this error is non-critical, however if it persists, please run with admin permissions or disable RAM Limiter");

                CloseHandle(hProcess);
                continue;
            }

            // Setting CPU
            /*
            if (!SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS))
            {
                if (!instanceExitFlag.load())
                    handleErrorMessage("Happened while trying to set RAM Usage Limit, this error is non-critical, however if it persists, please run with admin permissions or disable RAM Limiter");
            
                CloseHandle(hProcess);
                continue;
            }
            */


            /*
            if (!SetInformationJobObject(hJob, JobObjectCpuRateControlInformation, &info, sizeof(info)))
            {
                handleErrorMessage("Happened while trying to set Information to Job Object, this error is non-critical, however if it persists, please run with admin permissions or disable CPU Limiter");
                CloseHandle(hProcess);
                continue;
            }
            */

            /*
            // Assigning Process to Job
            if (!AssignProcessToJobObject(hJob, hProcess))
            {
                handleErrorMessage("Happened while trying to assign proccess to job (CPU Usage Limit), this error is non-critical, however if it persists, please run with admin permissions or disable CPU Limiter");
                CloseHandle(hProcess);
                continue;
            }
            */

            // Close Handle
            CloseHandle(hProcess);
        }
    }
}

// Credits to Unknown for clipboard script and pressCtrl
void setClipboardText(const std::string& text)
{
    if (!OpenClipboard(nullptr)) return;
    EmptyClipboard();

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (!hMem)
    {
        CloseClipboard();
        return;
    }

    memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
    GlobalUnlock(hMem);
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

// Allow focus
void focusOnHwnd(HWND hwnd)
{
    if (!IsWindow(hwnd)) return;

    ShowWindow(hwnd, SW_RESTORE);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    while (true) {
        if (GetForegroundWindow() == hwnd)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

// Convert LPCWSTR buffer to string (credits to chatgpt u came in cluch icl saved my ahh)
std::string convertInputText(HWND input)
{
    const int BUF_SIZE = 1024;
    wchar_t buffer[BUF_SIZE] = { 0 };
    std::string chatText;

    if (!input) {
        return "This feature isnt currently working!!!";
    }

    SendMessageW(input, WM_GETTEXT, BUF_SIZE, (LPARAM)buffer);

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
    if (size_needed > 0) {
        chatText.resize(size_needed);
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &chatText[0], size_needed, NULL, NULL);

        if (!chatText.empty() && chatText.back() == '\0') {
            chatText.pop_back();
        }
    }

    return chatText;
}

// Convert String to WString
std::wstring stringToWString(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstrTo[0], size_needed);
    return wstrTo;
}

// Bring back UI
void bringBackUI()
{
    ShowWindow(g_MainWindow, SW_RESTORE); 
    SetForegroundWindow(g_MainWindow);
}

// Spam Message Action
void spamMessageAction(int& durationMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    pressSlash();
    pressCtrlV();
    pressEnter();
}


// Case where it sends one message
void sendSingleMessage(std::vector<HWND>& windows, const std::string& emotes, const std::string& msg, const std::string& user)
{
    std::string greetings[] = {
        "yo wassup", "hola amigos", "salut les gars", "holla homies", "ciao belli",
        "hej hej", "aloha dude", "konnichiwa senpai", "wassup bro", "yo yo yo",
        "guten tag mates", "hej frens", "hola que pasa", "oi oi oi", "namaste fam"
    };

    std::vector<std::string> memHackLines = {
    "*(int*)0x0 = 1;",
    "*(char**)0x4;",
    "std::cout << wstr()",
    "addr = nullptr;",
    "val = 0x1000;",
    "Virtual_Alloc(*int ptr)"
    };

    std::vector<std::string> languageTroll = {
    "wrrwarawrawr",
    "rawsdafawtawtrawt",
    "wrwasfdasrfawawdfawf",
    "Hello, and welcome to Bambou, how may I help you!",
    "rwadsafatawtawtawt",
    "rwadfcas3estrawfasetgatawrAWR"
    };

    std::uniform_int_distribution<> distDelay(0, 14);
    std::uniform_int_distribution<> hack(0, 5);
    std::uniform_int_distribution<> languge(0, 5);

    // Handle emotes that just need to be sent as one command
    if (!emotes.empty() && emotes != "wave")
    {
        setClipboardText("/e " + emotes);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    else if (msg == "Language Troll")
    {
        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd);

            setClipboardText(languageTroll[languge(gen)]);
            std::this_thread::sleep_for(std::chrono::milliseconds(40));

            pressSlash();
            pressCtrlV();
            pressEnter();
        }

        pressCtrlV(true);

        return;
    }
    else if (msg == "Hacking")
    {
        setClipboardText("[HACKING]....");
        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd);

            pressSlash();
            pressCtrlV();
            pressEnter();
        }

        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd);

            std::string hackingMsg = memHackLines[hack(gen)];
            setClipboardText(hackingMsg);
            std::this_thread::sleep_for(std::chrono::milliseconds(40));

            pressSlash();
            pressCtrlV();
            pressEnter();
        }
        pressCtrlV(true);

        return; // Skip the rest
    }
    else if (!emotes.empty() && emotes == "wave")
    {
        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd);

            int greetIndex = distDelay(gen);
            std::string greet = greetings[greetIndex];
            setClipboardText(greet);

            std::this_thread::sleep_for(std::chrono::milliseconds(40));

            pressSlash();
            pressCtrlV();
            pressEnter();
        }

        setClipboardText("/e hello");
        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd);

            pressSlash();
            pressCtrlV();
            pressEnter();
        }

        pressCtrlV(true);

        return; // Skip rest
    }
    else if (!user.empty() && emotes.empty())
    {
        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd);

            std::string pmCommand = "/w " + user;
            setClipboardText(pmCommand);
            std::this_thread::sleep_for(std::chrono::milliseconds(40));

            pressSlash();
            pressCtrlV();
            pressEnter();
        }

        setClipboardText(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd);

            pressSlash();
            pressCtrlV();
            pressEnter();

            pressSlash();
            pressDelete();
            pressEnter();
        }

        pressCtrlV(true);

        return; // Skip the rest
    }
    else
    {
        // For custom messages
        setClipboardText(msg);
    }

    for (HWND hwnd : windows)
    {
        focusOnHwnd(hwnd);
        pressSlash();
        pressCtrlV();
        pressEnter();
    }
}


// Reset all Characters
void resetAllChars()
{
    const int delayBetweenClients = 5;
    const int wHoldPerClient = 20;

    std::vector<HWND> windows = getMoonWindows();
    for (HWND client : windows)
    {
        focusOnHwnd(client);
        pressEscape();
        pressR();
        pressEnter();
        pressEnter();
    }
}

// Spam Messages
void spamMessages(int durationMs, const std::string emotes, const std::string msg)
{
    if (emotes == "Dance")
    {
        setClipboardText("/e dance");
        spamMessageAction(durationMs);
        setClipboardText("/e dance3");
        spamMessageAction(durationMs);
    }
    else
    {
        setClipboardText(msg);
    }
    spamMessageAction(durationMs);
}

// Undance
void Undance(std::vector<HWND> windows, int durationMs)
{
    for (HWND hwnd : windows)
    {
        focusOnHwnd(hwnd);
        pressSpace();
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    }
}

// Sidebar update
void sideBarUpdater(HWND hwnd)
{
    std::vector<HWND> previousAmountOfInstances;
    std::vector<DWORD> previousAmountOfPids;

    while (true)
    {
        // Essential varibles
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::vector<HWND> RbxInstancesToRename = getRobloxWindows();
        for (HWND& hwnd : RbxInstancesToRename)
        {
            if (!hwnd) continue;
            BOOL blackEnabled = TRUE;
            HRESULT change = DwmSetWindowAttribute(hwnd, 20, &blackEnabled, sizeof(blackEnabled));
            if (FAILED(change))
                HRESULT change = DwmSetWindowAttribute(hwnd, 19, &blackEnabled, sizeof(blackEnabled));

            SetWindowText(hwnd, L"MoonRBX V1");
        }

        std::vector<HWND> instances = getMoonWindows();
        std::vector<DWORD> pids = getPids(instances);
        std::vector<DWORD> removedPids;
        std::vector<HWND> addedClients;
        std::map<HWND, DWORD> formattedAddedClients;

        // Find added windows
        for (HWND& hwnd : instances)
        {
            if (std::find(previousAmountOfInstances.begin(), previousAmountOfInstances.end(), hwnd) == previousAmountOfInstances.end())
            {
                if (masterClient == NULL)
                    masterClient = hwnd;

                addedClients.push_back(hwnd);
            }
        }

        // Find removed windows
        for (size_t x = 0; x < previousAmountOfInstances.size(); ++x)
        {
            HWND hwnd = previousAmountOfInstances[x];
            if (std::find(instances.begin(), instances.end(), hwnd) == instances.end())
            {
                if (hwnd == masterClient && listOfConnectedClients.size() != 0)
                    masterClient = listOfConnectedClients[0];
                else if (hwnd == masterClient && listOfConnectedClients.size() == 0)
                    masterClient = NULL;

                removedPids.push_back(previousAmountOfPids[x]);
            }
        }

        // Gui updates
        if (!addedClients.empty())
        {
            for (HWND hwnd : addedClients)
            {
                DWORD pid;
                GetWindowThreadProcessId(hwnd, &pid);
                formattedAddedClients[hwnd] = pid;
            }

            addNewUserThread(formattedAddedClients);
            PostMessage(hwnd, WM_UPDATE_SIDEBAR, 0, 0);
        }

        if (!removedPids.empty())
        {
            removeUserThread(removedPids);
            PostMessage(hwnd, WM_UPDATE_SIDEBAR, 0, 0);
        }

        // Previous amount update
        previousAmountOfInstances = instances;
        previousAmountOfPids = pids;
    }
}

// Suspend Threads
bool freezeThreadOperation(DWORD pid, bool suspend, const std::vector<DWORD>& optionalPids = {})
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    GUITHREADINFO guiSize = { sizeof(GUITHREADINFO) };

    if (snapshot == INVALID_HANDLE_VALUE || snapshot == NULL)
    {
        CloseHandle(snapshot);
        handleErrorMessage("Happened while trying to suspend Roblox Thread. Please run program as admin, or reinstall it.");
        return false;
    }

    // Storing all threads into an array
    THREADENTRY32 threadEntry = {};
    threadEntry.dwSize = sizeof(threadEntry);

    if (!Thread32First(snapshot, &threadEntry))
    {
        CloseHandle(snapshot);
        handleErrorMessage("Happened while trying to search for first WIN32 Thread. Please run program as admin, or reinstall it.");
        return false;
    }

    // Function to unsuspend or suspend thread
    auto suspendThreadsProcess = [&threadEntry, suspend] {
        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, false, threadEntry.th32ThreadID);
        if (hThread) {
            if (suspend)
                SuspendThread(hThread);
            if (!suspend)
                ResumeThread(hThread);
            CloseHandle(hThread);
        }

        else {
            std::cerr << "Failed to open thread ID: " << threadEntry.th32ThreadID << "\n";
        }
    };

    // Looping through all threads, suspending / opening them && checking if its a gui thread
    do
    {
        if (optionalPids.size() > 0 && pid == NULL)
        {
            auto it = std::find(optionalPids.begin(), optionalPids.end(), threadEntry.th32OwnerProcessID);
            if (it != optionalPids.end())
                suspendThreadsProcess();
            continue;
        }

        if (threadEntry.th32OwnerProcessID == pid && !GetGUIThreadInfo(threadEntry.th32ThreadID, &guiSize) && pid != NULL)
            suspendThreadsProcess();
    } while (Thread32Next(snapshot, &threadEntry));

    CloseHandle(snapshot);
    return true;
}

// Detect if Shift+e for freezing client
bool detectProcessEndForFreezing(const std::vector<DWORD> &pidsCleaned)
{
    // Detecting
    if (exitFlag.load()) 
    { 
        // Looping through the pids and unfreezing them
        freezeThreadOperation(NULL, false, pidsCleaned); 

        // bringing back UI
        ShowWindow(g_MainWindow, SW_RESTORE); 
        SetForegroundWindow(g_MainWindow); 
         
        return true; 
    }
    return false;
}

// Freze
void freezeAllClients(bool freeze)
{
    std::vector<HWND> windows = getMoonWindows();
    std::vector<DWORD> pids = getPids(windows);
    size_t numOfClients = windows.size();
    bool firstClient = true;

    if (numOfClients <= 1)
    {
        MessageBox(NULL, L"Error: You need more than one client open.", L"Error", MB_ICONERROR);
        return;
    }

    const int freezeDurationMs = 13000;
    int unfreezeDurationMs = 1500;
    auto restoreWindows = [&windows]{
        for (HWND client : windows)
        {
            ShowWindow(client, SW_RESTORE);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    while (true)
    {
        windows = getMoonWindows();
        pids = getPids(windows);
        windows.erase(windows.begin());
        pids.erase(pids.begin());

        if (detectProcessEndForFreezing(pids)) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(unfreezeDurationMs));

        for (HWND hwnd : windows)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            DWORD pid = 0;
            DWORD currentThreadPid = GetCurrentProcessId();
            if (detectProcessEndForFreezing(pids))
            {
                restoreWindows();
                return;
            }
            
            GetWindowThreadProcessId(hwnd, &pid);
            focusOnHwnd(hwnd);
            pressSpace();
            ShowWindow(hwnd, SW_MINIMIZE);

            std::this_thread::sleep_for(std::chrono::milliseconds(130));
            freezeThreadOperation(pid, true);
        }

        for (int x = 0; x < freezeDurationMs; x += 100)
        {
            if (detectProcessEndForFreezing(pids))
            {
                restoreWindows();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        freezeThreadOperation(NULL, false, pids);
        restoreWindows();
    }
}


// Main function
void mainFunc(const std::string& Mode, const bool& emote)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // Preset Variables
    std::vector<std::string> actionMap = {"Walk", "Walk Backward", "Walk Right", "Walk Left", "Jump"};
    std::vector<std::string> walkingMethods = { "Walk", "Walk Backward", "Walk Right", "Walk Left" };
    std::vector<std::tuple<HWND, HANDLE, HANDLE>> windows = getAllUserThreadInfo();
    std::vector<std::tuple<HWND, HANDLE, HANDLE>> temp;
    std::vector<std::tuple<HWND, HANDLE, HANDLE>> temp2;
    std::vector<HWND> windows2 = getMoonWindows();
    std::function<void()> chosenAction;
    INPUT movement = {};
    INPUT up = {};

    // Fill in information
    //searchUserThread(1, (double)2.0, nullptr, windows2);

    // Other Variables
    size_t count = windows.size();
    bool walkingStopped = false;
    bool walking = false;

    // Error Handling
    if (count == 0)
    {
        handleErrorMessage("No roblox clients found.");  
        bringBackUI();
        return;
    }

    // Handling case where action has to be peformed multiple times (Determining which one it is)
    auto it = std::find(actionMap.begin(), actionMap.end(), Mode);
    auto executeMovement = [&movement, &up]()
    {
         SendInput(1, &up, sizeof(INPUT));
         std::this_thread::sleep_for(std::chrono::milliseconds(4));
         SendInput(1, &movement, sizeof(INPUT));
         std::this_thread::sleep_for(std::chrono::milliseconds(40));
    };

    // Identifying what mode it is (if task has to be peformed in a loop)
    if (it != actionMap.end()) {
        if (Mode != "Spin" && Mode != "Spam")
        {
            walking = true;

            if (Mode == "Walk")
            {
                movement.type = INPUT_KEYBOARD;
                movement.ki.wVk = 'W';
                movement.ki.wScan = MapVirtualKey('W', MAPVK_VK_TO_VSC);
            }
            else if (Mode == "Walk Right")
            {
                movement.type = INPUT_KEYBOARD;
                movement.ki.wVk = 'D';
                movement.ki.wScan = MapVirtualKey('D', MAPVK_VK_TO_VSC);
            }
            else if (Mode == "Walk Left")
            {
                movement.type = INPUT_KEYBOARD;
                movement.ki.wVk = 'A';
                movement.ki.wScan = MapVirtualKey('A', MAPVK_VK_TO_VSC);
            }
            else if (Mode == "Walk Backward")
            {
                movement.type = INPUT_KEYBOARD;
                movement.ki.wVk = 'S';
                movement.ki.wScan = MapVirtualKey('S', MAPVK_VK_TO_VSC);
            }
            else if (Mode == "Jump")
            {
                movement.type = INPUT_KEYBOARD;
                movement.ki.wVk = VK_SPACE;
                movement.ki.wScan = MapVirtualKey(VK_SPACE, MAPVK_VK_TO_VSC);
            }

            movement.ki.dwFlags = KEYEVENTF_SCANCODE;
            up = movement;
            up.ki.dwFlags |= KEYEVENTF_KEYUP;
            chosenAction = executeMovement;
        }
    } 
    else if (Mode == "Spam")
    {
        chosenAction = std::bind(spamMessages, 30, "", convertInputText(hChatTextbox));
    }
    else if (Mode == "Spin")
    {
        chosenAction = std::bind(spamMessages, 30, "", convertInputText(hChatTextbox));
    }

    // Actions which are only peformed once or twice
    if (emote) { sendSingleMessage(windows2, Mode, "", ""); bringBackUI(); pressCtrlV(true); return; }
    if (Mode == "Send Message") { sendSingleMessage(windows2, "", convertInputText(hChatTextbox), ""); bringBackUI(); pressCtrlV(true); return; }
    if (Mode == "Hacking") { sendSingleMessage(windows2, "", "Hacking", ""); bringBackUI(); pressCtrlV(true); return; }
    if (Mode == "Language Troll") { sendSingleMessage(windows2, "", "Language Troll", ""); bringBackUI(); pressCtrlV(true); return; }
    if (Mode == "Private Message") { sendSingleMessage(windows2, "", convertInputText(hChatTextbox), convertInputText(hPmUserTextbox)); bringBackUI(); pressCtrlV(true); return; }
    if (Mode == "Undance") { Undance(windows2, 35); bringBackUI(); return; }
    if (Mode == "Wave") { sendSingleMessage(windows2, "Wave", "", ""); bringBackUI(); pressCtrlV(true); return; }
    if (Mode == "Reset") { resetAllChars(); bringBackUI(); return; }
    if (Mode == "Freeze") { freezeAllClients(true); bringBackUI(); return; }

    while (true)
    {
        // Actions that are peformed until you click e + shift
        for (size_t i = 0; i < windows.size(); i++)
        {
            if (!IsWindow(std::get<0>(windows[i]))) return;
            if (i + 1 < windows.size()) {
                temp.push_back(windows[i + 1]);
            }
            if (i + 2 < windows.size()) {
                temp.push_back(windows[i + 2]);
            }
            if (!temp.empty()) {
                applyJobsToUserThreads(temp, false);
            }
            if (i > 0) {
                temp2.push_back(windows[i - 1]);
                applyJobsToUserThreads(temp2, true);
            }

            focusOnHwnd(std::get<0>(windows[i]));
            chosenAction();

            if (exitFlag.load() && std::find(walkingMethods.begin(), walkingMethods.end(), Mode) != walkingMethods.end())
            {
                walkingStopped = true;
            }

            temp.clear();
            temp2.clear();
        }

        SendInput(1, &up, sizeof(INPUT));
        return;

        // Make clients stop at the same time. (gracefully)
        if (walkingStopped) 
        {
            SendInput(1, &up, sizeof(INPUT));
            bringBackUI(); applyJobsToUserThreads(windows, false); return;
        }
        else if (exitFlag.load())
        {
            SendInput(1, &up, sizeof(INPUT));
            bringBackUI(); applyJobsToUserThreads(windows, false); return;
        }
    }

    applyJobsToUserThreads(windows, false);
}


// apply UI settings
void applyUISettings(HWND& gui)
{
    listOfAllUI.push_back(gui);
    SendMessage(gui, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true, 0));
}

// apply UI settings
void applyUILabelSettings(HWND gui)
{
    listOfAllUI.push_back(gui);
    SendMessage(gui, WM_SETFONT, (WPARAM)hFontGreetings, MAKELPARAM(true, 0));
}

// Handle Settings Mode
void loadSettingsUI()
{
    // Destroy all Current UI
    for (HWND uiElement : listOfAllUI)
    {
        DestroyWindow(uiElement);
    }

    // Variables
    listOfAllUI = {};
    const int BUTTON_WIDTH = 60;
    const int BUTTON_HEIGHT = 20;
    const int BUTTON_SPACING_X = 15;
    const int BUTTON_SPACING_Y = 10;
    const int TEXTBOX_HEIGHT = 30;
    const int TEXTBOX_WIDTH = BUTTON_WIDTH * 3 + BUTTON_SPACING_X * 2;
    int CURRENT_Y = 20;

    // Insert all textboxs and button
    auto placeButton = [&](LPCWSTR text, PCWSTR input, int id)
        {
            HWND label = CreateWindow(L"STATIC", text, WS_CHILD | WS_VISIBLE, 20, CURRENT_Y, 250, 20, g_MainWindow, (HMENU)id, g_hInstance, NULL);
            applyUISettings(label);

            CURRENT_Y += 25;

            if (wcscmp(text, L"[ROBLOX] CPU Usage Limiter:") == 0)
            {
                hCpuUsage = CreateWindow(L"BUTTON", input, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, CURRENT_Y, 100, 20, g_MainWindow, (HMENU)1001, g_hInstance, NULL);
                applyUISettings(hCpuUsage);
                CURRENT_Y += 30;
            }

            if (wcscmp(text, L"[ROBLOX] Ram Usage Limiter:") == 0)
            {
                hRamUsage = CreateWindow(L"BUTTON", input, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, CURRENT_Y, 100, 20, g_MainWindow, (HMENU)1002, g_hInstance, NULL);
                applyUISettings(hRamUsage);
                CURRENT_Y += 30;
            }

            if (wcscmp(text, L"[Moon] High Performance:") == 0)
            {
                hPeformance = CreateWindow(L"BUTTON", input, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, CURRENT_Y, 100, 20, g_MainWindow, (HMENU)1002, g_hInstance, NULL);
                applyUISettings(hPeformance);
                CURRENT_Y += 30;
            }

            if (wcscmp(text, L"[Moon] Update:") == 0)
            {
                int buttonX = 25;
                int buttonY = CURRENT_Y;

                HWND button = CreateWindow(L"BUTTON", L"Update", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                    buttonX, buttonY, BUTTON_WIDTH + 10, BUTTON_HEIGHT + 10, g_MainWindow, (HMENU)ID_UPDATE, g_hInstance, NULL);

                applyUISettings(button);

                CURRENT_Y += BUTTON_HEIGHT + 30;
            }

            if (wcscmp(text, L"[Moon] High Performance:") == 0)
            {
                SendMessage(hCpuUsage, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hRamUsage, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hPeformance, BM_SETCHECK, BST_CHECKED, 0);

                HWND nameLabel = CreateWindow(L"STATIC", L"[Moon] Windows User Name:", WS_CHILD | WS_VISIBLE, 20, CURRENT_Y, 250, 20, g_MainWindow, (HMENU)1003, g_hInstance, NULL);
                applyUISettings(nameLabel);

                CURRENT_Y += 30;

                int textboxX = 20;
                int textboxY = CURRENT_Y;
                std::wstring userConverted(USER_NAME.begin(), USER_NAME.end());

                hName = CreateWindow(L"EDIT", userConverted.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                    textboxX, textboxY, TEXTBOX_WIDTH, TEXTBOX_HEIGHT, g_MainWindow, (HMENU)ID_NAME, g_hInstance, NULL);
                applyUISettings(hName);

                CURRENT_Y += TEXTBOX_HEIGHT + 10;

                int buttonX = textboxX;
                int buttonY = CURRENT_Y;

                HWND hConfirm = CreateWindow(L"BUTTON", L"Confirm", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                    buttonX, buttonY, BUTTON_WIDTH + 40, BUTTON_HEIGHT + 10, g_MainWindow, (HMENU)ID_CONFIRM, g_hInstance, NULL);
                applyUISettings(hConfirm);

                CURRENT_Y += BUTTON_HEIGHT + 30;
            }
        };


    // Place Buttons
    std::vector<std::tuple<LPCWSTR, LPCWSTR, int>> buttons;
    buttons.emplace_back(L"[ROBLOX] CPU Usage Limiter:", L"Enabled", ID_CPU_USAGE);
    buttons.emplace_back(L"[ROBLOX] Ram Usage Limiter:", L"Enabled", ID_CPU_USAGE);
    buttons.emplace_back(L"[Moon] High Performance:", L"Enabled", ID_PEFORMANCE);
    buttons.emplace_back(L"[Moon] Update:", L"Launch Updater", ID_PEFORMANCE);

    for (int x = 0; x < (int)buttons.size(); x++)
    {
        auto& buttonTuple = buttons[x];
        LPCWSTR inputText = std::get<0>(buttonTuple);
        LPCWSTR buttonText = std::get<1>(buttonTuple);
        int buttonID = std::get<2>(buttonTuple);

        placeButton(inputText, buttonText, buttonID);
    }
}


// Load normal UI
void loadNormalUI(bool onOpened)
{
    // Loading settings case
    if (!onOpened)
    {
        bool hCpuResult = SendMessage(hCpuUsage, BM_GETCHECK, 0, 0) == BST_CHECKED;
        bool hPeformanceResult = SendMessage(hPeformance, BM_GETCHECK, 0, 0) == BST_CHECKED;
        std::string hNameResult = convertInputText(hName);
        editValues(hNameResult, true, hPeformanceResult);

        CPU_LIMITOR = true;
        PEFORMANCE_MODE = hPeformanceResult;
        USER_NAME = hNameResult;

        for (HWND uiElement : listOfAllUI)
        {
            DestroyWindow(uiElement);
        }
        listOfAllUI = {};
    }

    // Loading Main UI && Main Variables
    const int BUTTON_WIDTH = 135;
    const int BUTTON_HEIGHT = 32;
    const int BUTTON_SPACING_X = 11;
    const int BUTTON_SPACING_Y = 5;
    const int START_X = 20;
    const int START_Y = 65;

    const int TEXTBOX_HEIGHT = 50;
    const int TEXTBOX_WIDTH = BUTTON_WIDTH * 3 + BUTTON_SPACING_X * 2;

    int x = START_X;
    int y = START_Y;
    int buttonsInRow = 0;

    // Loading Settings 
    HWND settingsIcon = CreateWindow(L"BUTTON", L"⚙", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 20, 10, 35, 35, g_MainWindow, (HMENU)ID_SETTINGS, g_hInstance, NULL);
    applyUISettings(settingsIcon);

    // Moon Lag Switch
    HWND lagIcon = CreateWindow(L"BUTTON", L"LS", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 60, 10, 35, 35, g_MainWindow, (HMENU)ID_LAG, g_hInstance, NULL);
    applyUISettings(lagIcon);

    // Formatting Username from STD::STRING to W::STRING too LPCWSTR, long asf ik
    std::wstring userConverted(USER_NAME.begin(), USER_NAME.end());
    std::wstring message = std::format(L"Welcome, {}!", userConverted);
    LPCWSTR formattedMsg = message.c_str();

    // Loading User
    HWND helloLabel = CreateWindow(L"STATIC", formattedMsg, WS_CHILD | WS_VISIBLE | SS_LEFT, 130, 10, 510, 40, g_MainWindow, NULL, g_hInstance, NULL);
    applyUILabelSettings(helloLabel);

    // Loading other buttons
    auto placeButton = [&](LPCWSTR label, int id)
    {
            HWND button = CreateWindow(L"BUTTON", label,
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                x, y, BUTTON_WIDTH, BUTTON_HEIGHT,
                g_MainWindow, (HMENU)id, g_hInstance, NULL);

            applyUISettings(button);

            buttonsInRow++;
            x += BUTTON_WIDTH + BUTTON_SPACING_X;

            if (buttonsInRow == 3)
            {
                buttonsInRow = 0; x = START_X; y += BUTTON_HEIGHT + BUTTON_SPACING_Y;
            }

            if (label == L"Evacuate!")
            {
                int textboxX = START_X;
                int textboxY = y + BUTTON_HEIGHT + BUTTON_SPACING_Y + 2;

                hChatTextbox = CreateWindow(L"EDIT", L"Enter Message", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, textboxX, textboxY, TEXTBOX_WIDTH, TEXTBOX_HEIGHT + 5, g_MainWindow, (HMENU)1001, g_hInstance, NULL);
                hPmUserTextbox = CreateWindow(L"EDIT", L"Enter User", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, textboxX, textboxY + 70, TEXTBOX_WIDTH, 35, g_MainWindow, (HMENU)1002, g_hInstance, NULL);

                int buttonX = textboxX + TEXTBOX_WIDTH + BUTTON_SPACING_X;
                int buttonY = textboxY;

                HWND broadcastButton = CreateWindow(L"BUTTON", L"Broadcast", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonX + 7, buttonY - 3, BUTTON_WIDTH + 43, BUTTON_HEIGHT - 4, g_MainWindow, (HMENU)ID_SEND_MESSAGE, g_hInstance, NULL);
                HWND spamButton = CreateWindow(L"BUTTON", L"Spam", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonX + 7, buttonY + BUTTON_HEIGHT * 0.8 + 3, BUTTON_WIDTH + 43, BUTTON_HEIGHT - 4, g_MainWindow, (HMENU)ID_SPAM_MESSAGE, g_hInstance, NULL);
                HWND privateMessageButton = CreateWindow(L"BUTTON", L"Private Msg", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonX + 7, buttonY + (BUTTON_HEIGHT) * 2 + 6, BUTTON_WIDTH + 43, BUTTON_HEIGHT - 2, g_MainWindow, (HMENU)ID_PM_MESSAGE, g_hInstance, NULL);

                applyUISettings(broadcastButton);
                applyUISettings(spamButton);
                applyUISettings(privateMessageButton);
                applyUISettings(hChatTextbox);
                applyUISettings(hPmUserTextbox);
            }
    };

    // Place Buttons
    std::vector<std::pair<LPCWSTR, int>> buttons = {
       {L"Walk", ID_WALK},
       {L"Walk Back", ID_WALK_BACKWARD},
       {L"Walk Right", ID_WALK_RIGHT},
       {L"Walk Left", ID_WALK_LEFT},
       {L"Jump", ID_JUMP},
       {L"Reset", ID_RESET},
       {L"NOT WORKING", ID_SPIN},
       {L"Dance", ID_DANCE},
       {L"Dance 2", ID_DANCE2},
       {L"Dance 3", ID_DANCE3},
       {L"Undance", ID_UNDANCE},
       {L"Point", ID_POINT},
       {L"Freeze", ID_FREEZE},
       {L"Stadium", ID_STADIUM},
       {L"Laugh", ID_LAUGH},
       {L"Shrug", ID_SHRUG},
       {L"Cheer", ID_CHEER},
       {L"Hello Troll", ID_WAVE},
       {L"Lang Troll", ID_LANGUAGE_TROLL},
       {L"Hack Troll", ID_HACKING},
       {L"Evacuate!", ID_EVACUATE}
    };

    for (auto& btn : buttons)
    {
        placeButton(btn.first, btn.second);
    }
}


// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        if (operationRunning)
            break;

        case ID_WALK:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk"), false).detach();
            operationRunning = false;
            break;
        case ID_WALK_RIGHT:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Right"), false).detach();
            operationRunning = false;
            break;
        case ID_WALK_LEFT:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Left"), false).detach();
            operationRunning = false;
            break;
        case ID_WALK_BACKWARD:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Backward"), false).detach();
            operationRunning = false;
            break;
        case ID_JUMP:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Jump"), false).detach();
            operationRunning = false;
            break;
        case ID_SLOW_WALK:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Slow Walk"), false).detach();
            operationRunning = false;
            break;
        case ID_SEND_MESSAGE:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Send Message"), false).detach();
            operationRunning = false;
            break;
        case ID_SPAM_MESSAGE:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Spam"), false).detach();
            operationRunning = false;
            break;
        case ID_DANCE:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("dance"), true).detach();
            operationRunning = false;
            break;
        case ID_DANCE2:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("dance2"), true).detach();
            operationRunning = false;
            break;
        case ID_DANCE3:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("dance3"), true).detach();
            operationRunning = false;
            break;
        case ID_DANCELOOP:
            operationRunning = true;
            ShowWindow(hwnd, SW_MINIMIZE);
            MessageBox(NULL, L"Error: This feature is currently broken. Please check back in later. Luv u - noah (no homo)", L"Error", MB_ICONERROR);
            bringBackUI();
            operationRunning = false;
            break;
        case ID_UNDANCE:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Undance"), false).detach();
            operationRunning = false;
            break;
        case ID_POINT:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("point"), true).detach();
            operationRunning = false;
            break;
        case ID_WAVE:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("wave"), true).detach();
            operationRunning = false;
            break;
        case ID_FREEZE:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Freeze"), false).detach();
            operationRunning = false;
            break;
        case ID_SHRUG:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("shrug"), true).detach();
            operationRunning = false;
            break;
        case ID_LAUGH:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("laugh"), true).detach();
            operationRunning = false;
            break;
        case ID_STADIUM:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("stadium"), true).detach();
            operationRunning = false;
            break;
        case ID_CHEER:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("cheer"), true).detach();
            operationRunning = false;
            break;
        case ID_PM_MESSAGE:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Private Message"), false).detach();
            operationRunning = false;
            break;
        case ID_LANGUAGE_TROLL:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Language Troll"), false).detach();
            operationRunning = false;
            break;
        case ID_HACKING:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Hacking"), false).detach();
            operationRunning = false;
            break;
        case ID_RESET:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Reset"), false).detach();
            operationRunning = false;
            break;
        case ID_SPIN:
            operationRunning = true;
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Spin"), false).detach();
            operationRunning = false;
            break;

        case ID_EVACUATE: {
            std::vector<HWND> windows = getMoonWindows();
            std::vector<DWORD> pids = getPids(windows);
            std::thread(notifyRemovalOfThread).detach();

            for (DWORD pid : pids)
            {
                if (pid == 0) continue;

                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, pid);
                if (hProcess != NULL)
                {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
                else
                {
                    handleErrorMessage("While trying to terminate process.");
                }
            }
            MessageBox(NULL, L"Evacuated all accounts!", L"Evacuation time...", MB_ICONINFORMATION);
            break;
        }
        case ID_LAG: {
            std::vector<HWND> windows = getMoonWindows();
            size_t count = windows.size();
            bool success = true;
            
            if (count == 0)
            {
                handleErrorMessage("No roblox clients found.");
                break;
            }

            getAdminPerms(success);
            if (!success)
            {
                handleErrorMessage("Denied admin permissions, cannot use Moon Lag Switch without admin permissions.");
                break;
            }

            if (!warningSent)
            {
                MessageBoxW(hwnd, L"Before enabling Moon Lag Switch, if you have any other anti-viruses excluding windows defender, ensure you disable their firewall. Otherwise it won't allow this program to interact with the windows firewall.", MB_OK, MB_ICONEXCLAMATION);
                warningSent = true;
            }

            (lagEnabled) ? enableAndDisableLagSwitch(false) : enableAndDisableLagSwitch(true);
            (lagEnabled) ? lagEnabled = false : lagEnabled = true;
            break;
        }
        case ID_SETTINGS:
            mode = "Settings";
            PostMessage(hwnd, VM_HANDLE_MODE, 0, 0);
            break;
        case ID_CONFIRM:
            mode = "Normal";
            PostMessage(hwnd, VM_HANDLE_MODE, 0, 0);
            break;
        case ID_UPDATE:
        {
            exit(0);
        }
        default:
            break;
        }

        return 0; 

    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_DESTROY: 
    {
        if (hBrushDarkGrey)
        {
            DeleteObject(hBrushDarkGrey);
            hBrushDarkGrey = NULL;
        }

        if (hFont) DeleteObject(hFont);
        if (hFontGreetings) DeleteObject(hFontGreetings);

        PostQuitMessage(0);
        return 0;
    }
    case WM_CREATE:
    {
        hBrushDarkGrey = CreateSolidBrush(RGB(40, 40, 40));
        break;
    }
    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam; if (lpdis->CtlType == ODT_BUTTON) 
        { 
            // Start Drawing process
            FillRect(lpdis->hDC, &lpdis->rcItem, hBackgroundBrush); 
            HBRUSH brushToUse = hButtonBrush;
            
            // Hover on buttons color change
            if (lpdis->itemState && lpdis->CtlID == ID_LAG && lagEnabled == false)
                brushToUse = hLagEnabledBrush;
            else if (lpdis->itemState && lpdis->CtlID == ID_LAG && lagEnabled == true)
                brushToUse = hLagDisabledBrush;
            else if (lpdis->itemState & ODS_SELECTED) 
                brushToUse = hButtonHoverBrush; 
            else if (lpdis->itemState & ODS_HOTLIGHT) 
                brushToUse = hButtonHoverBrush;

            // Draw the buttons
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(40,40,40)); 
            HPEN oldPen = (HPEN)SelectObject(lpdis->hDC, pen); 
            HBRUSH oldBrush = (HBRUSH)SelectObject(lpdis->hDC, brushToUse); 
            RoundRect(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right, lpdis->rcItem.bottom, 23, 23); 
            SetBkMode(lpdis->hDC, TRANSPARENT); 
            SetTextColor(lpdis->hDC, RGB(230, 230, 230)); 
            wchar_t text[256]; 
            GetWindowText(lpdis->hwndItem, text, _countof(text)); 
            DrawText(lpdis->hDC, text, -1, (LPRECT)&lpdis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE); 

            if (lpdis->itemState & ODS_FOCUS) 
                DrawFocusRect(lpdis->hDC, &lpdis->rcItem); 
            
            // Cleanup 
            SelectObject(lpdis->hDC, oldBrush);
            SelectObject(lpdis->hDC, oldPen); 
            DeleteObject(pen); 
            return true; 
   
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        if (!hBrushDarkGrey) break;
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(40, 40, 40));
        SetTextColor(hdc, RGB(255, 255, 255));
        return (INT_PTR)hBrushDarkGrey;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, hBrushDarkGrey);
        EndPaint(hwnd, &ps);

        return 0;
    }

    case VM_HANDLE_MODE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        if (mode == "Settings")
            loadSettingsUI();
        if (mode == "Normal")
            loadNormalUI(false);
    }
    case WM_UPDATE_SIDEBAR:
    {
        // Update Sidebar everytime new client appears/disappears
        RECT rc;
        GetClientRect(hwnd, &rc);
        const int EDIT_WIDTH = 162;
        const int EDIT_HEIGHT = 29;
        const int EDIT_MARGIN = 6;
        int margin = 5;

        int y = 20;
        int x = rc.right - EDIT_WIDTH - margin;

        if (listOfConnectedClients.size() != 0)
        {
            for (HWND instance : listOfConnectedClients)
            {
                DestroyWindow(instance);
            }
        }

        std::vector<HWND> robloxInstances = getMoonWindows();

        for (size_t index = 0; index < robloxInstances.size(); index++)
        {
            if (index >= 8)
            {
                std::wstring text2 = L"....Roblox Client #" + std::to_wstring(robloxInstances.size());
                HWND clientDisplay2 = CreateWindow(L"BUTTON", text2.c_str(), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, x, y, EDIT_WIDTH, EDIT_HEIGHT, hwnd, NULL, g_hInstance, NULL);
                listOfConnectedClients.push_back(clientDisplay2);
                SendMessage(clientDisplay2, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true, 0));

                break;
            }
            else if (index >= 8 && robloxInstances[index] == masterClient)
            {
                std::wstring text = L"[MASTER CLIENT]";
                HWND clientDisplay = CreateWindow(L"BUTTON", text.c_str(), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, x, y, EDIT_WIDTH, EDIT_HEIGHT, hwnd, NULL, g_hInstance, NULL);
                listOfConnectedClients.push_back(clientDisplay);
                SendMessage(clientDisplay, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true, 0));
                char title[256];
                char title2[256];

                GetWindowTextA(masterClient, title, sizeof(title));
                GetWindowTextA(robloxInstances[index], title2, sizeof(title2));
                handleErrorMessage(title);
                handleErrorMessage(title2);

            }
            else
            {
                std::wstring text = L"Roblox Client #" + std::to_wstring(index + 1);
                HWND clientDisplay = CreateWindow(L"BUTTON", text.c_str(), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, x, y, EDIT_WIDTH, EDIT_HEIGHT, hwnd, NULL, g_hInstance, NULL);
                listOfConnectedClients.push_back(clientDisplay);
                SendMessage(clientDisplay, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true, 0));
            }
            y += 30;
        }
    }
  }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* p) {
    
    MessageBox(NULL, L"Program crashed. Input has been unblocked.", L"Crash Handler", MB_ICONERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    SetUnhandledExceptionFilter(CrashHandler);
    readRequiredFiles();
    const wchar_t CLASS_NAME[] = L"SimpleWin32GUI";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    g_hInstance = hInstance;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST,
        CLASS_NAME,
        L"Moon V7 - By Noah",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 670, 520
        ,
        NULL, NULL, hInstance, NULL
    );

    std::thread(exitDetection).detach(); // Run Exit Detector
    std::thread(sideBarUpdater, hwnd).detach(); // Run SideBar Manager UI
    std::thread(optimisePeformance).detach(); // Run SideBar Manager UI

    g_MainWindow = hwnd;
    BOOL blackEnabled = TRUE;

    if (!hwnd) return 0;

    loadNormalUI(true);
    HRESULT change = DwmSetWindowAttribute(hwnd, 20, &blackEnabled, sizeof(blackEnabled));
    if (FAILED(change))
        HRESULT change = DwmSetWindowAttribute(hwnd, 19, &blackEnabled, sizeof(blackEnabled));

    MessageBoxW(hwnd,
        L"Moon Client Controller V7\n\nCreated by: Noah\nGitHub: https://github.com/thanknoah/Moon-Instance-Controller\nSpecial Thanks: justarandom (eygptian arab man)",
        L"Credits",
        MB_OK | MB_ICONINFORMATION);
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
