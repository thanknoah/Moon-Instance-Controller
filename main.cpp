// GUI credits to unknown.e, Code Logic Written by Noah

// imports
#include <random>
#include <tuple>
#include <mutex>
#include <format>
#include <Windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string>
#include <set>
#include <tlhelp32.h>
#include <filesystem>
//#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <uxtheme.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <wingdi.h>

using json = nlohmann::json;

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "UxTheme.lib")

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
#define WM_UPDATE_SIDEBAR (WM_USER + 1)
#define VM_HANDLE_MODE (WM_UPDATE_SIDEBAR + 1)

// Other Public Variables
std::atomic<bool> exitFlag(false);
std::atomic<bool> instanceExitFlag(false);
std::mutex cpuLimitMutex;
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
HINSTANCE g_hInstance = NULL;

// Other Global Variables
std::vector<HWND> listOfConnectedClients = {};
std::vector<HWND> listOfAllUI = {};
std::map<std::string, DWORD> err_msgs = {};

// Client Information
std::string mode = "Normal";
std::string CLIENT_VERSION = "V5";
std::string USER_NAME;

bool CPU_LIMITOR = false; // Default values
bool NEW_USER = false;
bool PEFORMANCE_MODE = false;

// UI Color
HBRUSH hBrushDarkGrey = NULL;
HBRUSH hButtonBrush = CreateSolidBrush(RGB(60, 60, 60));
HBRUSH hButtonHoverBrush = CreateSolidBrush(RGB(90, 150, 200));
HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(40, 40, 40));


// Handling error messages
void handleErrorMessage(const std::string& msg)
{
    DWORD err = GetLastError();
    std::string errCode = "[Error CODE " + std::to_string(err) + "]: " + msg;
    std::wstring formattedErrCode(errCode.begin(), errCode.end());
    MessageBox(NULL, formattedErrCode.c_str(), L"Error", MB_ICONERROR);
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

// Get total amount of Roblox Instances Open
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


            return TRUE;
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


// Sleep simplified function [Seconds]
void sleepSec(int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

// Detect if exit is clicked (ctrl + e)
void exitDetection()
{
    bool wasPressed = false;

    while (true)
    {
        bool detectEPressed = (GetAsyncKeyState('E') & 0x8000);
        bool detectShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
        bool combination = detectEPressed && detectShiftPressed;

        if (combination && !wasPressed)
        {
            exitFlag.store(true);
            sleepSec(1);
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

// Structure
struct userProcessDetails
{
    BOOL isRunning;
    DOUBLE cpuCap;
    DOUBLE cpuUsage;
    DWORD pid;
};

// Thread storage
std::mutex userThreadMutex;
std::vector<std::shared_ptr<userProcessDetails>> userProcessDetailsList;

// Smart RAM Allocater
bool getCpuUsageInfoForAllClients(DWORD pid, std::shared_ptr<userProcessDetails> newDetails)
{
    // Essential Variables
    std::unique_lock<std::mutex> lock(userThreadMutex);
    //std::vector<DOUBLE> cpuUsageCollections;
    userProcessDetailsList.push_back(newDetails);
    lock.unlock();

    // Amount of Processors
    SYSTEM_INFO sys;
    GetSystemInfo(&sys);
    int numOfProccesors = sys.dwNumberOfProcessors;


    // Main Loop
    while (newDetails->isRunning)
    {
        // Opening Process
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SET_INFORMATION | PROCESS_SET_QUOTA, TRUE, pid);
        if (hProcess == NULL)
            continue;
        
        try {
            // Process info
            FILETIME fileCreation, fileExit, fileKernel, fileUser;
            ULARGE_INTEGER creationTime1, exitTime1, kernelTime1, userTime1;

            GetProcessTimes(hProcess, &fileCreation, &fileExit, &fileKernel, &fileUser);
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

            GetProcessTimes(hProcess, &fileCreation2, &fileExit2, &fileKernel2, &fileUser2);
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

            // Calculations
            if (sysTotalDiff > 0)
            {
                cpuPercent = double(procTotalDiff) / (double)sysTotalDiff * 100.0;
            }
            else
            {
                CloseHandle(hProcess);
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
            if (pid != 0 && hProcess == NULL)
                CloseHandle(hProcess);
            continue;
        }
        CloseHandle(hProcess);
    }
}


// New userThread
void addNewUserThread(const std::vector<DWORD>& pids)
{
    for (DWORD pid : pids)
    {
        auto newDetails = std::make_shared<userProcessDetails>();
        newDetails->cpuUsage = 0.0;
        newDetails->cpuCap = 0.0;
        newDetails->isRunning = TRUE;
        newDetails->pid = pid;

        std::unique_lock<std::mutex> lock(userThreadMutex);
        userProcessDetailsList.emplace_back(newDetails);
        std::thread(getCpuUsageInfoForAllClients, pid, newDetails).detach();
        lock.unlock();
    }
}

// Search userThread
BOOL searchUserThread(DWORD pidSearch, double& var, std::string type)
{
    std::unique_lock<std::mutex> lock(userThreadMutex);
    for (auto it = userProcessDetailsList.begin(); it != userProcessDetailsList.end(); ++it)
    {
        if ((*it)->pid == pidSearch)
        {
            if (type == "cpuUsage")
            {
                var = (*it)->cpuUsage;
                if ((*it)->cpuUsage == 0.0) return FALSE;
                return TRUE;
            } 
            else if (type == "cpuCap")
            {
                var = (*it)->cpuCap;
                if ((*it)->cpuCap == 0.0) return FALSE;
                return TRUE;
            }   
        }
    }
    lock.unlock();
    return FALSE;
}

// Alert
void notifyRemovalOfThread()
{
    instanceExitFlag.store(TRUE);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    instanceExitFlag.store(FALSE);
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
                userProcessDetailsList.erase(it);
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
    // Sys
    SYSTEM_INFO sys;
    GetSystemInfo(&sys);
    double numOfProcessors = (double)sys.dwNumberOfProcessors;
    std::vector<HANDLE> jobs;

    while (true)
    {
        // Variables
        std::vector<HWND> listOfClients = getRobloxWindows();
        std::vector<DWORD> pids = getPids(listOfClients);
        size_t pidSize = pids.size();

        // Skip
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        if (listOfClients.size() == 0) continue;

        // Memory info
        DWORDLONG availPhys = 0;
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);

        // Getting Available Memory
        if (GlobalMemoryStatusEx(&memInfo))
            availPhys = memInfo.ullAvailPhys * 0.7;

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
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SET_INFORMATION | PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, pid);
            if (hProcess == NULL)
            {
                handleErrorMessage("Happened while trying to open Roblox Process ID, this error is non-critical, however if it persists, please run with admin permissions or disable CPU Limiter");
                if (pid != 0)
                    CloseHandle(hProcess);
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
                const size_t MAX_RAM = 500ULL * 1024 * 1024; // 500 MB


                // Weight
                size_t baseAllocation = availPhys / pidSize;  // Equal share of physical RAM
                size_t adjustedAllocation = static_cast<size_t>(baseAllocation * (100-cpuUsage));

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

            // Setting RAM
            if (!SetProcessWorkingSetSizeEx(hProcess, lowerRamLimit, upperRamLimit, 0) && !instanceExitFlag)
            {
                handleErrorMessage("Happened while trying to set RAM Usage Limit, this error is non-critical, however if it persists, please run with admin permissions or disable RAM Limiter");
                CloseHandle(hProcess);
                continue;
            }

            // Setting CPU
            if (!SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS) && !instanceExitFlag)
            {
                handleErrorMessage("Happened while trying to set CPU Usage Limit, this error is non-critical, however if it persists, please run with admin permissions or disable RAM Limiter");
                CloseHandle(hProcess);
                continue;
            }


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

// Low-Level Keyboard Emulator
void pressW(int durationMs)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = 'W'; // W key
    down.ki.wScan = MapVirtualKey('W', MAPVK_VK_TO_VSC);
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    SendInput(1, &up, sizeof(INPUT));
}


// Press S
void pressS(int durationMs)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = 'S'; // W key
    down.ki.wScan = MapVirtualKey('S', MAPVK_VK_TO_VSC);
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    SendInput(1, &up, sizeof(INPUT));
}

// Press A
void pressA(int durationMs)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = 'A';
    down.ki.wScan = MapVirtualKey('A', MAPVK_VK_TO_VSC);
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    SendInput(1, &up, sizeof(INPUT));
}

// Press D
void pressD(int durationMs)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = 'D';
    down.ki.wScan = MapVirtualKey('D', MAPVK_VK_TO_VSC);
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    SendInput(1, &up, sizeof(INPUT));
}
// Press sapce
void pressSpace(int durationMs)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = VK_SPACE; // Spacebar
    down.ki.wScan = MapVirtualKey(VK_SPACE, MAPVK_VK_TO_VSC);
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    SendInput(1, &up, sizeof(INPUT));
}

// Press delete
void pressDelete(int durationMs)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = 0; // Using scan code only
    down.ki.wScan = MapVirtualKey(VK_BACK, MAPVK_VK_TO_VSC);
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    SendInput(1, &up, sizeof(INPUT));
}

// Press Slash
void pressSlash(int durationMs)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = 0; // We are using scan code directly
    down.ki.wScan = MapVirtualKey(VK_OEM_2, MAPVK_VK_TO_VSC); // Slash key `/`
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    SendInput(1, &up, sizeof(INPUT));
}

// Credits to Unknown for clipboard script and pressCtrl
void setClipboardText(const std::string text)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    if (size_needed == 0) return;

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size_needed * sizeof(wchar_t));
    if (!hMem) return;

    wchar_t* wstr = (wchar_t*)GlobalLock(hMem);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wstr, size_needed);
    GlobalUnlock(hMem);

    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, hMem);
        CloseClipboard();
    }
    else {
        GlobalFree(hMem); // Clean up if clipboard couldn't be opened
    }
}

void pressCtrlV(int durationMs)
{
    INPUT inputs[4] = {};

    // Press CTRL
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 0;
    inputs[0].ki.wScan = MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;

    // Press V
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 0;
    inputs[1].ki.wScan = MapVirtualKey('V', MAPVK_VK_TO_VSC);
    inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE;

    // Release V
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 0;
    inputs[2].ki.wScan = MapVirtualKey('V', MAPVK_VK_TO_VSC);
    inputs[2].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    // Release CTRL
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = 0;
    inputs[3].ki.wScan = MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
    inputs[3].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    // Send input
    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    // Optional sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
}


// Press Enter
void pressEnter(int durationMs)
{
    INPUT down = {};
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = VK_RETURN;
    down.ki.wScan = MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC);
    down.ki.dwFlags = KEYEVENTF_SCANCODE;

    INPUT up = down;
    up.ki.dwFlags |= KEYEVENTF_KEYUP;

    SendInput(1, &down, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    SendInput(1, &up, sizeof(INPUT));
}

// Suspend Threads
bool freezeThreadOperation(DWORD pid, bool suspend, const std::vector<DWORD>& optionalPids = {})
{
    // Taking Screenshot Of Threads
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE || snapshot == NULL)
    {
        CloseHandle(snapshot);
        handleErrorMessage("Happened while trying to suspend Roblox Thread. Please run program as admin, or reinstall it.");
        return FALSE;
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

    auto suspendThreadsProcess = [&threadEntry, suspend]{
        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadEntry.th32ThreadID);
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
        GUITHREADINFO guiCheck = { sizeof(GUITHREADINFO) };

        if (optionalPids.size() > 0 && pid == NULL)
        {
            auto it = std::find(optionalPids.begin(), optionalPids.end(), threadEntry.th32OwnerProcessID);
            if (it != optionalPids.end())
               suspendThreadsProcess();
            continue;
        }

        if (threadEntry.th32OwnerProcessID == pid && !GetGUIThreadInfo(threadEntry.th32ThreadID, &guiCheck) && pid != NULL) 
            suspendThreadsProcess();
    } while (Thread32Next(snapshot, &threadEntry));

    CloseHandle(snapshot);
    return true;
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

// Allow focus
void focusOnHwnd(HWND hwnd, bool walking = false, int setTime = 0)
{
    if (!IsWindow(hwnd)) return;

    ShowWindow(hwnd, SW_RESTORE); 
    BringWindowToTop(hwnd); 
    SetForegroundWindow(hwnd); 
    SetActiveWindow(hwnd); 
    SetFocus(hwnd);

    int time;
    while (true) {
        if (GetForegroundWindow() == hwnd)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
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
    pressSlash(durationMs);
    pressCtrlV(durationMs);
    pressEnter(durationMs);
}


// Case where it sends one message
void sendSingleMessage(std::vector<HWND> windows, const std::string& emotes, const std::string& msg, const std::string& user)
{
    const int wHoldPerClient = 21;
    const int delayBetweenClients = 6;

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
            DWORD currentThread = GetCurrentThreadId();
            DWORD targetThread = GetWindowThreadProcessId(hwnd, NULL);
            AttachThreadInput(currentThread, targetThread, TRUE);
            focusOnHwnd(hwnd, false);

            setClipboardText(languageTroll[languge(gen)]);
            std::this_thread::sleep_for(std::chrono::milliseconds(40));

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            AttachThreadInput(currentThread, targetThread, FALSE);

            std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenClients));
        }
        
        return;
    }
    else if (msg == "Hacking")
    {
        setClipboardText("[HACKING]....");
        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        for (HWND hwnd : windows)
        {
            DWORD currentThread = GetCurrentThreadId();
            DWORD targetThread = GetWindowThreadProcessId(hwnd, NULL);

            AttachThreadInput(currentThread, targetThread, TRUE);
            focusOnHwnd(hwnd, false);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            AttachThreadInput(currentThread, targetThread, FALSE);

            
            std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenClients));
        }

        for (HWND hwnd : windows)
        {
            DWORD currentThread = GetCurrentThreadId();
            DWORD targetThread = GetWindowThreadProcessId(hwnd, NULL);
            AttachThreadInput(currentThread, targetThread, TRUE);
            focusOnHwnd(hwnd, false);

            std::string hackingMsg = memHackLines[hack(gen)];
            setClipboardText(hackingMsg);
            std::this_thread::sleep_for(std::chrono::milliseconds(40));

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            AttachThreadInput(currentThread, targetThread, FALSE);

            std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenClients));
        }

        return; // Skip the rest
    }
    else if (!emotes.empty() && emotes == "wave")
    {
        for (HWND hwnd : windows)
        {
            DWORD currentThread = GetCurrentThreadId();
            DWORD targetThread = GetWindowThreadProcessId(hwnd, NULL);
            AttachThreadInput(currentThread, targetThread, TRUE);
            focusOnHwnd(hwnd, false);

            int greetIndex = distDelay(gen);
            std::string greet = greetings[greetIndex];
            setClipboardText(greet);

            std::this_thread::sleep_for(std::chrono::milliseconds(40));

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            AttachThreadInput(currentThread, targetThread, FALSE);

            std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenClients));
        }

        setClipboardText("/e hello");
        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        for (HWND hwnd : windows)
        {
            DWORD currentThread = GetCurrentThreadId();
            DWORD targetThread = GetWindowThreadProcessId(hwnd, NULL);
            AttachThreadInput(currentThread, targetThread, TRUE);
            focusOnHwnd(hwnd, false);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            AttachThreadInput(currentThread, targetThread, FALSE);

            std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenClients));
        }

        
        return; // Skip rest
    }
    else if (!user.empty() && emotes.empty())
    {  
        for (HWND hwnd : windows)
        {
            DWORD currentThread = GetCurrentThreadId();
            DWORD targetThread = GetWindowThreadProcessId(hwnd, NULL);
            AttachThreadInput(currentThread, targetThread, TRUE);
            focusOnHwnd(hwnd, false);

            std::string pmCommand = "/w " + user;
            setClipboardText(pmCommand);
            std::this_thread::sleep_for(std::chrono::milliseconds(40)); 

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            AttachThreadInput(currentThread, targetThread, FALSE);

            std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenClients));
        }

        setClipboardText(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        for (HWND hwnd : windows)
        {
            DWORD currentThread = GetCurrentThreadId();
            DWORD targetThread = GetWindowThreadProcessId(hwnd, NULL);
            AttachThreadInput(currentThread, targetThread, TRUE);

            focusOnHwnd(hwnd, false);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);

            pressSlash(wHoldPerClient);
            pressDelete(wHoldPerClient);
            pressEnter(wHoldPerClient);
            AttachThreadInput(currentThread, targetThread, FALSE);
            

            std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenClients));
        }

        return; // Skip the rest
    }
    else
    {
        // For custom messages
        setClipboardText(msg);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(40));

    for (HWND hwnd : windows)
    {
        DWORD currentThread = GetCurrentThreadId();
        DWORD targetThread = GetWindowThreadProcessId(hwnd, NULL);
        AttachThreadInput(currentThread, targetThread, TRUE);

        focusOnHwnd(hwnd, false);        
        pressSlash(wHoldPerClient);
        pressCtrlV(wHoldPerClient);
        pressEnter(wHoldPerClient);
        AttachThreadInput(currentThread, targetThread, FALSE);

        std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenClients));
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
        pressSpace(durationMs);
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
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::vector<HWND> instances = getRobloxWindows();
        std::vector<DWORD> pids = getPids(instances);
        std::vector<DWORD> removedPids;
        std::vector<HWND> addedClients;

        // Find added windows
        for (HWND& hwnd : instances)
        {
            if (std::find(previousAmountOfInstances.begin(), previousAmountOfInstances.end(), hwnd) == previousAmountOfInstances.end())
            {
                addedClients.push_back(hwnd);
            }
        }

        // Find removed windows
        for (size_t x = 0; x < previousAmountOfInstances.size(); ++x)
        {
            HWND hwnd = previousAmountOfInstances[x];
            if (std::find(instances.begin(), instances.end(), hwnd) == instances.end())
            {
                removedPids.push_back(previousAmountOfPids[x]);
            }
        }

        // Gui updates
        if (!addedClients.empty())
        {
            addNewUserThread(getPids(addedClients));
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

// Detect if Shift+e for freezing client
bool detectProccesEnd(const std::vector<DWORD> &pidsCleaned)
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
    std::vector<HWND> windows = getRobloxWindows();
    std::vector<DWORD> pids = getPids(windows);
    size_t numOfClients = windows.size();
    bool firstClient = true;

    if (numOfClients <= 1)
    {
        MessageBox(NULL, L"Error: You need more than one client open.", L"Error", MB_ICONERROR);
        return;
    }

    const int freezeDurationMs = 13000;
    int unfreezeDurationMs = 500;

    while (true)
    {
        windows = getRobloxWindows();
        pids = getPids(windows);

        windows.erase(windows.begin());
        pids.erase(pids.begin());

        if (detectProccesEnd(pids)) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(unfreezeDurationMs));

        for (HWND hwnd : windows)
        {
            DWORD pid = 0;
            if (detectProccesEnd(pids)) return;
            
            GetWindowThreadProcessId(hwnd, &pid);
            focusOnHwnd(hwnd);
            pressSpace(15); 
            std::this_thread::sleep_for(std::chrono::milliseconds(145));

            freezeThreadOperation(pid, true);
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }

        for (int x = 0; x < freezeDurationMs; x += 100)
        {
            if (detectProccesEnd(pids)) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        for (DWORD pid : pids)
        {
            if (detectProccesEnd(pids)) return;
            freezeThreadOperation(NULL, false, pids);
        }
    }
}


// Main function
void mainFunc(const std::string& Mode, const bool& emote)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Preset Variables
    std::unordered_map<std::string, void(*)(int)> actionMap = { {"Walk", pressW}, {"Walk Backward", pressS}, {"Walk Right", pressD}, {"Walk Left", pressA}, {"Jump", pressSpace} };
    std::set<std::string> walkingMethods = { "Walk", "Walk Backward", "Walk Right", "Walk Left" };
    std::vector<HWND> windows = getRobloxWindows();
    std::vector<std::thread> threads;

    // Other Variables
    size_t count = windows.size();
    BOOL walkingStopped = FALSE;
    BOOL walking = FALSE;
    int counter = 0;

    // Error Handling
    if (count == 0)
    {
        handleErrorMessage("No roblox clients found.");  bringBackUI();
        return;
    }

    // Handling case where action has to be peformed multiple times (Determining which one it is)
    void (*normalfuncPtr)(int) = nullptr;
    void (*spamFuncPtr)(int, const std::string, const std::string) = nullptr;
    auto it = actionMap.find(Mode);

    // Identifying what mode it is
    if (it != actionMap.end()) {
        normalfuncPtr = it->second;

        if (Mode != "Jump" && Mode != "Undance")
        {
            walking = true;
        }
    }
    else if (Mode == "Spam") spamFuncPtr = spamMessages;

    // Walk Function
    auto walk = [&normalfuncPtr]()
    {
       for (int x = 0; x < 10; x++)
       {
           normalfuncPtr(1);
       }
    };

    // Actions which are only peformed once or twice
    if (emote) { sendSingleMessage(windows, Mode, "", ""); bringBackUI(); return; }
    if (Mode == "Send Message") { sendSingleMessage(windows, "", convertInputText(hChatTextbox), ""); bringBackUI(); return; }
    if (Mode == "Hacking") { sendSingleMessage(windows, "", "Hacking", ""); bringBackUI(); return; }
    if (Mode == "Language Troll") { sendSingleMessage(windows, "", "Language Troll", ""); bringBackUI(); return; }
    if (Mode == "Private Message") { sendSingleMessage(windows, "", convertInputText(hChatTextbox), convertInputText(hPmUserTextbox)); bringBackUI(); return; }
    if (Mode == "Undance") { Undance(windows, 35); bringBackUI(); return; }
    if (Mode == "Wave") { sendSingleMessage(windows, "Wave", "", ""); bringBackUI(); return; }
    if (Mode == "Freeze") { freezeAllClients(true); bringBackUI(); return; }
   
    while (true)
    {
        // Handling Exit mode
        if (exitFlag.load() && walkingMethods.find(Mode) != walkingMethods.end()) 
        { 
            walkingStopped = true; 
        } 
        else if (exitFlag.load()) 
        { 
            bringBackUI();  
            return; 
        }
        
        // Actions that are peformed until you click e + shift
        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd, walking);
            if (normalfuncPtr != nullptr)
            {
                if (walking)
                    walk();
                if (!walking) 
                    normalfuncPtr(20);
            }

            if (spamFuncPtr != nullptr) 
                spamFuncPtr(35, "None", convertInputText(hChatTextbox));

            if (exitFlag.load() && walkingMethods.find(Mode) != walkingMethods.end()) { walkingStopped = true; break; } 
            else if (exitFlag.load()) { bringBackUI(); return; }
        }

        // Make clients stop at the same time.
        if (walkingStopped) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            bringBackUI(); return;
        }
    }
}

// Font used for UI
HFONT hFont = CreateFontW(
    28, 0, 0, 0, FW_LIGHT,
    FALSE, FALSE, FALSE,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    CLEARTYPE_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE,
    L"Segoe UI"
);

HFONT hFontGreetings = CreateFont(33, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Calibri");

// apply UI settings
void applyUISettings(HWND gui)
{
    listOfAllUI.push_back(gui);
    SendMessage(gui, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

// apply UI settings
void applyUILabelSettings(HWND gui)
{
    listOfAllUI.push_back(gui);
    SendMessage(gui, WM_SETFONT, (WPARAM)hFontGreetings, MAKELPARAM(TRUE, 0));
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
    const int START_Y = 60;

    const int TEXTBOX_HEIGHT = 50;
    const int TEXTBOX_WIDTH = BUTTON_WIDTH * 3 + BUTTON_SPACING_X * 2;

    int x = START_X;
    int y = START_Y;
    int buttonsInRow = 0;

    // Loading Settings 
    HWND settingsIcon = CreateWindow(L"BUTTON", L"⚙", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 20, 10, 35, 35, g_MainWindow, (HMENU)ID_SETTINGS, g_hInstance, NULL);
    applyUISettings(settingsIcon);

    // Formatting Username from STD::STRING to W::STRING too LPCWSTR, long asf ik
    std::wstring userConverted(USER_NAME.begin(), USER_NAME.end());
    std::wstring message = std::format(L"Welcome, {}!", userConverted);
    LPCWSTR formattedMsg = message.c_str();

    // Loading User
    HWND helloLabel = CreateWindow(L"STATIC", formattedMsg, WS_CHILD | WS_VISIBLE | SS_LEFT, 80, 10, 510, 40, g_MainWindow, NULL, g_hInstance, NULL);
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
                int textboxY = y + BUTTON_HEIGHT + BUTTON_SPACING_Y + 20;

                hChatTextbox = CreateWindow(L"EDIT", L"Enter Message", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, textboxX, textboxY, TEXTBOX_WIDTH, TEXTBOX_HEIGHT + 5, g_MainWindow, (HMENU)1001, g_hInstance, NULL);
                hPmUserTextbox = CreateWindow(L"EDIT", L"Enter User", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, textboxX, textboxY + 70, TEXTBOX_WIDTH, 35, g_MainWindow, (HMENU)1002, g_hInstance, NULL);

                int buttonX = textboxX + TEXTBOX_WIDTH + BUTTON_SPACING_X;
                int buttonY = textboxY;

                HWND broadcastButton = CreateWindow(L"BUTTON", L"Broadcast", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonX+7, buttonY-3, BUTTON_WIDTH+29, BUTTON_HEIGHT-4, g_MainWindow, (HMENU)ID_SEND_MESSAGE, g_hInstance, NULL);
                HWND spamButton = CreateWindow(L"BUTTON", L"Spam", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonX+7, buttonY + BUTTON_HEIGHT*0.8 + 3, BUTTON_WIDTH+29, BUTTON_HEIGHT-4, g_MainWindow, (HMENU)ID_SPAM_MESSAGE, g_hInstance, NULL);
                HWND privateMessageButton = CreateWindow(L"BUTTON", L"Private Msg", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonX+7, buttonY + (BUTTON_HEIGHT)*2 + 3, BUTTON_WIDTH+29, BUTTON_HEIGHT-2, g_MainWindow, (HMENU)ID_PM_MESSAGE, g_hInstance, NULL);

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
       {L"Dance", ID_DANCE},
       {L"Dance 2", ID_DANCE2},
       {L"Dance 3", ID_DANCE3},
       {L"Zombie", ID_DANCELOOP},
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
        case ID_WALK:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk"), false).detach();
            break;
        case ID_WALK_RIGHT:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Right"), false).detach();
            break;
        case ID_WALK_LEFT:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Left"), false).detach();
            break;
        case ID_WALK_BACKWARD:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Backward"), false).detach();
            break;
        case ID_JUMP:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Jump"), false).detach();
            break;
        case ID_SLOW_WALK:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Slow Walk"), false).detach();
            break;
        case ID_SEND_MESSAGE:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Send Message"), false).detach();
            break;
        case ID_SPAM_MESSAGE:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Spam"), false).detach();
            break;
        case ID_DANCE:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("dance"), true).detach();
            break;
        case ID_DANCE2:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("dance2"), true).detach();
            break;
        case ID_DANCE3:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("dance3"), true).detach();
            break;
        case ID_DANCELOOP:
            ShowWindow(hwnd, SW_MINIMIZE);
            MessageBox(NULL, L"Error: This feature is currently broken. Please check back in later. Luv u - noah (no homo)", L"Error", MB_ICONERROR);
            bringBackUI();
            break;
        case ID_UNDANCE:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Undance"), false).detach();
            break;
        case ID_POINT:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("point"), true).detach();
            break;
        case ID_WAVE:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("wave"), true).detach();
            break;
        case ID_FREEZE:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Freeze"), false).detach();
            break;
        case ID_SHRUG:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("shrug"), true).detach();
            break;
        case ID_LAUGH:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("laugh"), true).detach();
            break;
        case ID_STADIUM:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("stadium"), true).detach();
            break;
        case ID_CHEER:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("cheer"), true).detach();
            break;
        case ID_PM_MESSAGE:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Private Message"), false).detach();
            break;
        case ID_LANGUAGE_TROLL:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Language Troll"), false).detach();
            break;
        case ID_HACKING:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Hacking"), false).detach();
            break;
        case ID_EVACUATE: {
            std::vector<HWND> windows = getRobloxWindows();
            std::vector<DWORD> pids = getPids(windows);

            for (DWORD pid : pids)
            {
                if (pid == 0) continue;

                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
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
            MessageBox(NULL, L"OMG! EVACUATE SOLDIERS, EVACUATE. HOPE YOU DIDNT GET BANNED YA IBN KALB, haha hacked ur pc leaking ur ip 2424242424 JK LOL HAHA. but fr tho hope u didnt get banned : )", L"Crash Handler", MB_ICONINFORMATION);
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
            FillRect(lpdis->hDC, &lpdis->rcItem, hBackgroundBrush); 

            HBRUSH brushToUse = hButtonBrush; 
            if (lpdis->itemState & ODS_SELECTED) 
                brushToUse = hButtonHoverBrush; 
            else if (lpdis->itemState & ODS_HOTLIGHT) 
                brushToUse = hButtonHoverBrush;

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
            return TRUE; 
   
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
        const int EDIT_HEIGHT = 28;
        const int EDIT_MARGIN = 6;
        int margin = 3;

        int y = 20;
        int x = rc.right - EDIT_WIDTH - margin;

        if (listOfConnectedClients.size() != 0)
        {
            for (HWND instance : listOfConnectedClients)
            {
                DestroyWindow(instance);
            }
        }

        std::vector<HWND> robloxInstances = getRobloxWindows();
        size_t numOfClients = robloxInstances.size();

        for (size_t index = 0; index < numOfClients; index++)
        {

            if (index >= 9)
            {
                std::wstring text2 = L"....Roblox Client #" + std::to_wstring(numOfClients);
                HWND clientDisplay2 = CreateWindow(L"BUTTON", text2.c_str(), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, x, y, EDIT_WIDTH, EDIT_HEIGHT, hwnd, NULL, g_hInstance, NULL);
                listOfConnectedClients.push_back(clientDisplay2);
                SendMessage(clientDisplay2, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

                break;
            }
            else
            {
                std::wstring text = L"Roblox Client #" + std::to_wstring(index + 1);
                HWND clientDisplay = CreateWindow(L"BUTTON", text.c_str(), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, x, y, EDIT_WIDTH, EDIT_HEIGHT, hwnd, NULL, g_hInstance, NULL);
                listOfConnectedClients.push_back(clientDisplay);
                SendMessage(clientDisplay, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
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
        L"Moon Client Controller V5.5 - By Noah",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 670, 505,
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
        L"Moon Client Controller V5\n\nCreated by: Noah\nGitHub: https://github.com/thanknoah/Moon-Instance-Controller\nSpecial Thanks: justarandom (eygptian arab man)",
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
