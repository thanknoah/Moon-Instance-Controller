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
#include <sstream>
#include <string>
#include <set>
#include <tlhelp32.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "interception.h"

using json = nlohmann::json;

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
#define WM_UPDATE_SIDEBAR (WM_USER + 1)
#define VM_HANDLE_MODE (WM_UPDATE_SIDEBAR + 1)

// Other Public Variables
std::atomic<bool> exitFlag(false);
std::mutex cpuLimitMutex;
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distSleep(3, 20);

HWND g_MainWindow = NULL;
HWND hChatTextbox = NULL;
HWND hPmUserTextbox = NULL;
HWND hCpuUsage = NULL;
HWND hPeformance = NULL;
HWND hName = NULL;
HINSTANCE g_hInstance = NULL;

std::vector<HWND> listOfConnectedClients = {};
std::vector<HWND> listOfAllUI = {};

std::string mode = "Normal";
std::string UP_TO_DATE_VER = "V4";
std::string USER_NAME;
std::string VERSION;

bool CPU_LIMITOR = false; // Default values
bool NEW_USER = false;
bool PEFORMANCE_MODE = false;

HBRUSH hBrushDarkGrey = NULL;

// Fix self attempt
void fixFilesAttempt(bool fileExists)
{
    MessageBox(NULL, L"An error occured while trying to access info.json.. Will attempt to fix..", L"Error", MB_ICONERROR);
    try 
    {
        if (fileExists)
           std::filesystem::remove("info.json");

        json j;
        j["NEW_USER"] = false;
        j["CPU_LIMITOR"] = true;
        j["PEFORMANCE_MODE"] = false;
        j["USER_NAME"] = "Guest";
        j["VERSION"] = "V4";

        std::ofstream outFile("info.json");
        outFile << j.dump(4);
        outFile.close();
        MessageBox(NULL, L"Fixed file! Please reopen program.", L"Error", MB_ICONINFORMATION);
        exit(0);

    }
    catch (...)
    {
        MessageBox(NULL, L"Error trying to interact info.json, please try reinstalling program", L"Error", MB_ICONERROR);
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
            j["VERSION"] = "V4";
            MessageBox(NULL, L"It appears your new. Welcome Guest, to V4 Moon Instance Controller! The program will now adjust a few things, then you can get right into it.", L"Greeting", MB_ICONINFORMATION);

            std::ofstream outFile("info.json");
            outFile << j.dump(4);
            outFile.close();
        }
        catch (...)
        {
            MessageBox(NULL, L"Error: Lack of administration permissions occured while trying to access info.json, please run as administrator!", L"Error", MB_ICONERROR);
            exit(0);
        }
    }

    std::ifstream inFile("info.json");
    if (!inFile)
    {
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
            MessageBox(NULL, L"Error: Lack of administration permissions occured while trying to access info.json, please run as administrator!", L"Error", MB_ICONERROR);
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
        if (!j.contains("VERSION") || !j["VERSION"].is_string()) fixFilesAttempt(false);

        NEW_USER = j["NEW_USER"];
        PEFORMANCE_MODE = j["PEFORMANCE_MODE"];
        CPU_LIMITOR = j["CPU_LIMITOR"];
        USER_NAME = j["USER_NAME"];
        VERSION = j["VERSION"];

        if (VERSION != UP_TO_DATE_VER)
        {
            MessageBox(NULL, L"New update is out! Please go to settings and click 'update'.", L"Greeting", MB_ICONINFORMATION);
        }

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
        j["VERSION"] = "V4";

        std::ofstream outFile("info.json");
        outFile << j.dump(4);
        outFile.close();
    }
    catch (...)
    {
        MessageBox(NULL, L"Error: Lack of administration permissions occured while trying to access info.json, please run as administrator!", L"Error", MB_ICONERROR);
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
            BlockInput(FALSE);
        }
        else if (!combination)
        {
            wasPressed = false;
        }

        Sleep(5); // Run Every 5 milliseconds (detector)
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

// Limit CPU Usage
void constantLimitCpuAndRamUsage()
{
    while (true)
    {
        // Open the target process
        std::vector<HWND> listOfClients = getRobloxWindows();
        std::vector<DWORD> pids = getPids(listOfClients);

        for (DWORD pid : pids)
        {
            // Opening PID
            HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION | PROCESS_SET_QUOTA, FALSE, pid);

            // Error checking
            if (hProcess == NULL)
            {
                MessageBox(NULL, L"Failed to open the target process.", L"Error", MB_ICONERROR);
                continue;
            }

            // Setting CPU Limitations
            if (!SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS))
            {
                MessageBox(NULL, L"Failed to set CPU usage limit.", L"Error", MB_ICONERROR);
                continue;
            }

            // Assign RAM Limitations
            size_t lowerRamLimit = 1ULL * 1024 * 1024;
            size_t upperRamLimit = 2ULL * 1024 * 1024;

            if (!SetProcessWorkingSetSizeEx(hProcess, lowerRamLimit, upperRamLimit, 0))
            {
                MessageBox(NULL, L"Failed to set RAM usage limit.", L"Error", MB_ICONERROR);
                continue;
            }

            // Close Handle
            CloseHandle(hProcess);
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
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
void setClipboardText(const std::string& text)
{
    OpenClipboard(0);
    EmptyClipboard();
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (!hg) return;
    memcpy(GlobalLock(hg), text.c_str(), text.size() + 1);
    GlobalUnlock(hg);
    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();
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
bool manageThreadOperations(DWORD pid, bool suspend)
{
    // Taking Screenshot Of Threads
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, L"An error occured while trying to suspend thread operation: Please try again, or contact the devs.", L"Error", MB_ICONERROR);
        return false;
    }

    // Storing all threads into an array
    THREADENTRY32 threadEntry = {};
    threadEntry.dwSize = sizeof(threadEntry);

    if (!Thread32First(snapshot, &threadEntry))
    {
        CloseHandle(snapshot);
        MessageBox(NULL, L"An error occured while trying to find first thread: Please try again, or contact the devs.", L"Error", MB_ICONERROR);
        return false;
    }

    // Looping through all threads, suspending / opening them && checking if its a gui thread
    do
    {
        GUITHREADINFO guiCheck = { sizeof(GUITHREADINFO) };

        if (threadEntry.th32OwnerProcessID == pid && !GetGUIThreadInfo(threadEntry.th32ThreadID, &guiCheck)) {
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

        }
    } while (Thread32Next(snapshot, &threadEntry));

    CloseHandle(snapshot);
    return true;
}

// Convert Buffer to string (credits to chatgpt u came in cluch icl saved my ahh)
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
            chatText.pop_back();  // Clean up null terminator
        }
    }

    return chatText;
}

// Allow focus
void focusOnHwnd(HWND hwnd)
{
    if (!IsWindow(hwnd)) return;
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    SetActiveWindow(hwnd);
    Sleep(3);
}

void chatFocusOnHwnd(HWND hwnd)
{
    if (!IsWindow(hwnd)) return;
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    SetActiveWindow(hwnd);
}

// Bring back UI
void bringBackUI()
{
    ShowWindow(g_MainWindow, SW_RESTORE); 
    SetForegroundWindow(g_MainWindow); 
    BlockInput(FALSE);
}

// Spam Message Action
void spamMessageAction(int& durationMs)
{
    Sleep(durationMs);
    pressSlash(durationMs);
    pressCtrlV(durationMs);
    pressEnter(durationMs);
}


// Case where it sends one message
void sendSingleMessage(std::vector<HWND> windows, const std::string& emotes, const std::string& msg, const std::string& user)
{
    int wHoldPerClient = 18;
    int delayBetweenClients = 18;

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
    "write(0x1000, 1337);",
    };

    std::uniform_int_distribution<> distDelay(0, 14);
    std::uniform_int_distribution<> hack(0, 5);

    // Handle emotes that just need to be sent as one command
    if (!emotes.empty() && emotes != "wave")
    {
        setClipboardText("/e " + emotes);
        Sleep(50);
    }
    else if (msg == "Hacking")
    {
        setClipboardText("[HACKING]....");
        Sleep(50);

        for (HWND hwnd : windows)
        {
            chatFocusOnHwnd(hwnd);
            BlockInput(TRUE);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);

            BlockInput(FALSE);
            Sleep(delayBetweenClients);
        }

        for (HWND hwnd : windows)
        {
            focusOnHwnd(hwnd);
            std::string hackingMsg = memHackLines[hack(gen)];

            setClipboardText(hackingMsg);
            Sleep(50);

            BlockInput(TRUE);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);

            BlockInput(FALSE);
            Sleep(delayBetweenClients);
        }

        return; // Skip the rest
    }
    else if (!emotes.empty() && emotes == "wave")
    {
        BlockInput(TRUE);

        for (HWND hwnd : windows)
        {
            chatFocusOnHwnd(hwnd);

            int greetIndex = distDelay(gen);
            std::string greet = greetings[greetIndex];
            setClipboardText(greet);

            Sleep(50);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            Sleep(delayBetweenClients);
        }

        setClipboardText("/e hello");
        Sleep(50);

        for (HWND hwnd : windows)
        {

            chatFocusOnHwnd(hwnd);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            Sleep(delayBetweenClients);
        }

        BlockInput(FALSE);
        return; // Skip rest
    }
    else if (!user.empty() && emotes.empty())
    {  
        for (HWND hwnd : windows)
        {

            chatFocusOnHwnd(hwnd);
            std::string pmCommand = "/w " + user;
            setClipboardText(pmCommand);
            Sleep(50);

            BlockInput(TRUE);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);

            BlockInput(FALSE);

            Sleep(delayBetweenClients);
        }

        setClipboardText(msg);
        Sleep(50);

        for (HWND hwnd : windows)
        {

            chatFocusOnHwnd(hwnd);
            Sleep(wHoldPerClient);
            BlockInput(TRUE);

            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);

            pressSlash(wHoldPerClient);
            pressDelete(wHoldPerClient);
            pressEnter(wHoldPerClient);

            BlockInput(FALSE);

            Sleep(delayBetweenClients);
        }

        return; // Skip the rest
    }
    else
    {
        // For custom messages
        setClipboardText(msg);
    }

    Sleep(50);

    for (HWND hwnd : windows)
    {
        chatFocusOnHwnd(hwnd);
        Sleep(wHoldPerClient);
        BlockInput(TRUE);
        
        pressSlash(wHoldPerClient);
        pressCtrlV(wHoldPerClient);
        pressEnter(wHoldPerClient);

        BlockInput(FALSE);
        Sleep(delayBetweenClients);
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
        Sleep(durationMs);
    }
}

// Sidebar update
void sideBarUpdater(HWND hwnd)
{
    size_t currentAmountOfInstances = 0;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::vector<HWND> instances = getRobloxWindows();
        std::vector<HWND> instancesToAttach;
        if (instances.size() != currentAmountOfInstances)
        {
            PostMessage(hwnd, WM_UPDATE_SIDEBAR, 0, 0);

            if (instances.size() < currentAmountOfInstances)
            {
                currentAmountOfInstances = instances.size();
                continue;
            }

            size_t difference = instances.size() - currentAmountOfInstances;
            currentAmountOfInstances = instances.size();
        }
    }
}

// Detect if Shift+e for freezing client
bool detectProccesEnd(const std::vector<DWORD> &pidsCleaned)
{
    if (exitFlag.load()) 
    { 
        for (DWORD pid : pidsCleaned) 
            manageThreadOperations(pid, false); 
        ShowWindow(g_MainWindow, SW_RESTORE); 
        SetForegroundWindow(g_MainWindow); 
        BlockInput(FALSE); 
        return true; 
    }
    return false;
}

// Freze
void freezeAllClients(bool freeze)
{
    std::vector<HWND> windows = getRobloxWindows();
    std::set<DWORD> pids;
    size_t numOfClients = windows.size();
    bool firstClient = true;

    if (numOfClients <= 1)
    {
        MessageBox(NULL, L"Error: You need more than one client open.", L"Error", MB_ICONERROR);
        return;
    }

    if (freeze && !windows.empty())
        windows.erase(windows.begin());

    for (HWND hwnd : windows)
    {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);

        if (pid != 0)
            pids.insert(pid);
    }

    std::vector<DWORD> pidsCleaned(pids.begin(), pids.end());
    const int freezeDurationMs = 12000;
    int unfreezeDurationMs = 1000;

    while (true)
    {
        if (detectProccesEnd(pidsCleaned)) return;
        Sleep(unfreezeDurationMs);

        BlockInput(TRUE);

        for (HWND hwnd : windows)
        {
            DWORD pid = 0;
            if (detectProccesEnd(pidsCleaned)) return;
            GetWindowThreadProcessId(hwnd, &pid);
            focusOnHwnd(hwnd);

            Sleep(20); 
            pressSpace(20); 
            Sleep(110);

            manageThreadOperations(pid, true); // Freezes specfic client
            Sleep(50); // focus
            if (detectProccesEnd(pidsCleaned)) return;
        }

        BlockInput(FALSE);

        for (int x = 0; x < freezeDurationMs; x += 100)
        {
            if (detectProccesEnd(pidsCleaned)) return;
            Sleep(100);
        }

        for (DWORD pid : pidsCleaned)
            manageThreadOperations(pid, false);
        if (detectProccesEnd(pidsCleaned)) return;
    }
}

// Main function
void mainFunc(const std::string& Mode, const bool& emote)
{
    // Preset Variables
    Sleep(25);

    std::vector<HWND> windows = getRobloxWindows();
    std::vector<std::thread> threads;
    std::set<std::string> walkingMethods = { "Walk", "Walk Backward", "Walk Right", "Walk Left" };
    std::unordered_map<std::string, void(*)(int)> actionMap = { {"Walk", pressW}, {"Walk Backward", pressS}, {"Walk Right", pressD}, {"Walk Left", pressA}, {"Jump", pressSpace} };

    // Other Variables
    size_t count = windows.size();
    bool walkingStopped = false;
    int counter = 0;

    // Error Handling
    if (count == 0)
    {
        MessageBox(NULL, L"No Roblox clients found.", L"Error", MB_ICONERROR); BlockInput(FALSE); bringBackUI();
        return;
    }

    // Handling case where action has to be peformed multiple times
    void (*normalfuncPtr)(int) = nullptr;
    void (*spamFuncPtr)(int, const std::string, const std::string) = nullptr;
    auto it = actionMap.find(Mode);

    if (it != actionMap.end()) {
        normalfuncPtr = it->second;
    }
    else if (Mode == "Spam")
    {
        spamFuncPtr = spamMessages;
    }

    // Actions which are only peformed once or twice
    if (emote) { sendSingleMessage(windows, Mode, "", ""); bringBackUI(); return; }
    if (Mode == "Send Message") { sendSingleMessage(windows, "", convertInputText(hChatTextbox), ""); bringBackUI(); return; }
    if (Mode == "Hacking") { sendSingleMessage(windows, "", "Hacking", ""); bringBackUI(); return; }
    if (Mode == "Private Message") { sendSingleMessage(windows, "", convertInputText(hChatTextbox), convertInputText(hPmUserTextbox)); bringBackUI(); return; }
    if (Mode == "Undance") { Undance(windows, 35); bringBackUI(); return; }
    if (Mode == "Wave") { sendSingleMessage(windows, "Wave", "", ""); bringBackUI(); return; }
    if (Mode == "Freeze") { freezeAllClients(true); bringBackUI(); return; }
   
    // Actions that are peformed until you click e + shift
    threads.emplace_back([&]() {
        while (true) {
            if (exitFlag.load() && walkingMethods.find(Mode) != walkingMethods.end()) { walkingStopped = true; break; }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    for (size_t i = 0; i < windows.size(); ++i)
    {
        int initialDelay = 3 * static_cast<int>(i);  // stagger start

        if (normalfuncPtr != nullptr)
        {
            threads.emplace_back([hwnd_ = windows[i], normalsFuncPtr_ = normalfuncPtr, initialDelay_ = initialDelay, distSleep_ = distSleep, gen_ = gen]() mutable {
                std::this_thread::sleep_for(std::chrono::milliseconds(initialDelay_));
                while (!exitFlag.load()) {
                    focusOnHwnd(hwnd_);
                    normalsFuncPtr_(20);
                    int sleepDuration = distSleep_(gen_);
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
                }
                });
        }
        else
        {
            threads.emplace_back([hwnd_ = windows[i], spamFuncPtr_ = spamFuncPtr, initialDelay_ = initialDelay, distSleep_ = distSleep, gen_ = gen]() mutable {
                std::this_thread::sleep_for(std::chrono::milliseconds(initialDelay_));
                while (!exitFlag.load()) {
                    focusOnHwnd(hwnd_);
                    spamFuncPtr_(20, "None", convertInputText(hChatTextbox));
                    int sleepDuration = distSleep_(gen_);
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
                }
                });
        }
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    if (walkingStopped)
    {
        Sleep(10);
        bringBackUI(); return;
    }
    bringBackUI();  return;
}

// Font used for UI
HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Calibri");
HFONT hFontGreetings = CreateFont(30, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Calibri");

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
                HWND nameLabel = CreateWindow(L"STATIC", L"[Moon] Windows User Name:", WS_CHILD | WS_VISIBLE, 20, CURRENT_Y, 250, 20, g_MainWindow, (HMENU)1003, g_hInstance, NULL);
                applyUISettings(nameLabel);

                CURRENT_Y += 30;

                int textboxX = 20;
                int textboxY = CURRENT_Y;

                hName = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                    textboxX, textboxY, TEXTBOX_WIDTH, TEXTBOX_HEIGHT, g_MainWindow, (HMENU)ID_NAME, g_hInstance, NULL);
                applyUISettings(hName);

                CURRENT_Y += TEXTBOX_HEIGHT + 10;

                int buttonX = textboxX;
                int buttonY = CURRENT_Y;

                HWND hConfirm = CreateWindow(L"BUTTON", L"Confirm", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                    buttonX, buttonY, BUTTON_WIDTH + 40, BUTTON_HEIGHT + 10, g_MainWindow, (HMENU)ID_CONFIRM, g_hInstance, NULL);
                applyUISettings(hConfirm);

                CURRENT_Y += BUTTON_HEIGHT + 15;
            }
        };


    // Place Buttons
    std::vector<std::tuple<LPCWSTR, LPCWSTR, int>> buttons;
    buttons.emplace_back(L"[ROBLOX] CPU Usage Limiter:", L"Enabled", ID_CPU_USAGE);
    buttons.emplace_back(L"[Moon] Update:", L"Launch Updater", ID_PEFORMANCE);
    buttons.emplace_back(L"[Moon] High Performance:", L"Enabled", ID_PEFORMANCE);

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
        bool hCpuResult = (SendMessage(hCpuUsage, BM_GETCHECK, 0, 0) == BST_CHECKED);
        bool hPeformanceResult = (SendMessage(hPeformance, BM_GETCHECK, 0, 0) == BST_CHECKED);
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
    const int BUTTON_WIDTH = 120;
    const int BUTTON_HEIGHT = 30;
    const int BUTTON_SPACING_X = 15;
    const int BUTTON_SPACING_Y = 12;
    const int START_X = 20;
    const int START_Y = 60;

    const int TEXTBOX_HEIGHT = 50;
    const int TEXTBOX_WIDTH = BUTTON_WIDTH * 3 + BUTTON_SPACING_X * 2;

    int x = START_X;
    int y = START_Y;
    int buttonsInRow = 0;

    // Loading Settings 
    HWND settingsIcon = CreateWindow(L"BUTTON", L"⚙", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 20, 10, 30, 30, g_MainWindow, (HMENU)ID_SETTINGS, g_hInstance, NULL);
    applyUISettings(settingsIcon);


    // Formatting Username from STD::STRING to W::STRING too LPCWSTR, long asf ik
    std::wstring userConverted(USER_NAME.begin(), USER_NAME.end());
    std::wstring message = std::format(L"Welcome, {}!", userConverted);
    LPCWSTR formattedMsg = message.c_str();

    // Loading User
    HWND helloLabel = CreateWindow(L"STATIC", formattedMsg, WS_CHILD | WS_VISIBLE | SS_LEFT, 80, 10, 500, 30, g_MainWindow, NULL, g_hInstance, NULL);
    applyUILabelSettings(helloLabel);

    // Loading other buttons
    auto placeButton = [&](LPCWSTR label, int id)
        {
            HWND button = CreateWindow(L"BUTTON", label,
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                x, y, BUTTON_WIDTH, BUTTON_HEIGHT,
                g_MainWindow, (HMENU)id, g_hInstance, NULL);

            applyUISettings(button);

            buttonsInRow++;
            x += BUTTON_WIDTH + BUTTON_SPACING_X;

            if (buttonsInRow == 3)
            {
                buttonsInRow = 0; x = START_X; y += BUTTON_HEIGHT + BUTTON_SPACING_Y;
            }

            if (label == L"Hacking")
            {
                int textboxX = START_X;
                int textboxY = y + BUTTON_HEIGHT + BUTTON_SPACING_Y - 15;

                hChatTextbox = CreateWindow(L"EDIT", L"Enter Message", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, textboxX, textboxY, TEXTBOX_WIDTH, TEXTBOX_HEIGHT + 5, g_MainWindow, (HMENU)1001, g_hInstance, NULL);
                hPmUserTextbox = CreateWindow(L"EDIT", L"Enter User", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, textboxX, textboxY + 70, TEXTBOX_WIDTH, 30, g_MainWindow, (HMENU)1002, g_hInstance, NULL);

                int buttonX = textboxX + TEXTBOX_WIDTH + BUTTON_SPACING_X;
                int buttonY = textboxY;

                HWND broadcastButton = CreateWindow(L"BUTTON", L"Broadcast", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, buttonX, buttonY, BUTTON_WIDTH - 15, BUTTON_HEIGHT - 5, g_MainWindow, (HMENU)ID_SEND_MESSAGE, g_hInstance, NULL);
                HWND spamButton = CreateWindow(L"BUTTON", L"Spam", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, buttonX, buttonY + BUTTON_HEIGHT + 0.5, BUTTON_WIDTH - 15, BUTTON_HEIGHT - 5, g_MainWindow, (HMENU)ID_SPAM_MESSAGE, g_hInstance, NULL);
                HWND privateMessageButton = CreateWindow(L"BUTTON", L"Private Msg", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, buttonX, buttonY + BUTTON_HEIGHT + BUTTON_HEIGHT + 10, BUTTON_WIDTH - 15, BUTTON_HEIGHT - 5, g_MainWindow, (HMENU)ID_PM_MESSAGE, g_hInstance, NULL);

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
       {L"Walk (back)", ID_WALK_BACKWARD},
       {L"Walk (right)", ID_WALK_RIGHT},
       {L"Walk (left)", ID_WALK_LEFT},
       {L"Jump", ID_JUMP},
       {L"Dance", ID_DANCE},
       {L"Dance 2", ID_DANCE2},
       {L"Dance 3", ID_DANCE3},
       {L"Zombie Dance", ID_DANCELOOP},
       {L"Undance", ID_UNDANCE},
       {L"Wave", ID_WAVE},
       {L"Point", ID_POINT},
       {L"Freeze", ID_FREEZE},
       {L"Stadium", ID_STADIUM},
       {L"Laugh", ID_LAUGH},
       {L"Shrug", ID_SHRUG},
       {L"Cheer", ID_CHEER},
       {L"Hacking", ID_HACKING},
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
            MessageBox(NULL, L"Error: This feature is currently broken. Please check back in later. Luv u - noah (no homo)", L"Error", MB_ICONERROR);
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
        case ID_HACKING:
            focusOnHwnd(hwnd);
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Hacking"), false).detach();
            break;
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
            LPCWSTR exePath = L"Moon Installer.exe";
            ShellExecute(NULL, L"open", exePath, NULL, NULL, SW_SHOWNORMAL);
            exit(0);
        }
        default:
            break;
        }

        return 0; 

    case WM_CLOSE:
        BlockInput(FALSE);
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        if (hBrushDarkGrey)
        {
            DeleteObject(hBrushDarkGrey);
            hBrushDarkGrey = NULL;
        }

        if (hFont) DeleteObject(hFont);
        if (hFontGreetings) DeleteObject(hFontGreetings);

        PostQuitMessage(0);
        return 0;

    case WM_CREATE:
        hBrushDarkGrey = CreateSolidBrush(RGB(25, 25, 25));
        break;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC:
    {
        if (!hBrushDarkGrey) break;
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(25, 25, 25));
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
        const int EDIT_WIDTH = 190;
        const int EDIT_HEIGHT = 25;
        const int EDIT_MARGIN = 5;
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
            std::wstring text = L"Connected Client #" + std::to_wstring(index);
            LPCWSTR lpcwstr = text.c_str();

            HWND clientDisplay = CreateWindow(L"BUTTON", lpcwstr,
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                x, y, EDIT_WIDTH, EDIT_HEIGHT,
                hwnd, NULL, g_hInstance, NULL);

            listOfConnectedClients.push_back(clientDisplay);
            SendMessage(clientDisplay, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

            y += 30;
        }
    }
  }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* p) {
    BlockInput(FALSE);
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
        L"Moon Client Controller V4",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 665, 490,
        NULL, NULL, hInstance, NULL
    );

    std::thread(exitDetection).detach(); // Run Exit Detector
    std::thread(sideBarUpdater, hwnd).detach(); // Run SideBar Manager UI
    std::thread(constantLimitCpuAndRamUsage).detach(); // Run SideBar Manager UI

    g_MainWindow = hwnd;

    if (!hwnd) return 0;

    loadNormalUI(true);
    MessageBoxW(hwnd,
        L"Moon Client Controller V4\n\nCreated by: Noah\nGitHub: https://github.com/thanknoah/Moon-Instance-Controller\nSpecial Thanks: Unknown",
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


