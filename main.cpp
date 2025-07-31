// GUI credits to unknown.e, Code Logic Written by Noah

// imports
#include <random>
#include <Windows.h>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <set>
#include <tlhelp32.h>
#include <filesystem>

// Public Variables
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
#define ID_CHATBOX 18
#define ID_STADIUM 19
#define ID_CHEER 20
#define ID_LAUGH 21
#define ID_SHRUG 22
#define WM_UPDATE_SIDEBAR (WM_USER + 1)


std::atomic<bool> exitFlag(false);
std::random_device rd;
std::mt19937 gen(rd());
HWND g_MainWindow = NULL;
HWND hChatTextbox = NULL;
HINSTANCE g_hInstance = NULL;
std::vector<HWND> listOfUI = {};

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


// Sleep simplified function [Milliseconds]
void sleepMilli(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
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

        sleepMilli(10); // Run Every 10 milliseconds (detector)
    }
}

// Limit CPU Usage
bool limitCpuUsage(DWORD pid)
{
    // Open the target process
    HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL)
    {
        MessageBox(NULL, L"Failed to open the target process.", L"Error", MB_ICONERROR);
        return false;
    }

    // Create a Job Object
    HANDLE hJob = CreateJobObject(NULL, NULL);
    if (hJob == NULL)
    {
        MessageBox(NULL, L"Failed to create Job Object.", L"Error", MB_ICONERROR);
        CloseHandle(hProcess);
        return false;
    }

    // Set CPU rate cap
    JOBOBJECT_CPU_RATE_CONTROL_INFORMATION cpuRateInfo = {};
    cpuRateInfo.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_WEIGHT_BASED;
    cpuRateInfo.CpuRate = 1;

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
    info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    // Apply CPU rate limit
    if (!SetInformationJobObject(hJob, JobObjectCpuRateControlInformation, &cpuRateInfo, sizeof(cpuRateInfo)))
    {
        MessageBox(NULL, L"Failed to set CPU rate limit.", L"Error", MB_ICONERROR);
        CloseHandle(hJob);
        CloseHandle(hProcess);
        return false;
    }

    SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &info, sizeof(info));
    if (!AssignProcessToJobObject(hJob, hProcess))
    {
        MessageBox(NULL, L"Failed to assign process to Job Object.", L"Error", MB_ICONERROR);
        CloseHandle(hJob);
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}

// OnStart
void onStart(size_t n, std::vector<HWND> customizableInstances)
{
    std::vector<HWND> rblxInstances = getRobloxWindows();
    std::vector<DWORD> pidsCleaned;
    std::set<DWORD> pids;
    DWORD pid = 0;

    if (rblxInstances.size() == 0) return;
    if (n != 0) customizableInstances == rblxInstances;

    for (HWND instance : customizableInstances)
    {
        SetWindowPos(instance, NULL, 0, 0, 100, 100, SWP_NOZORDER | SWP_NOMOVE);
        GetWindowThreadProcessId(instance, &pid);
        if (pid != 0)
            pids.insert(pid);
    }

    pidsCleaned = std::vector<DWORD>(pids.begin(), pids.end());

    for (DWORD pid : pidsCleaned)
    {
        limitCpuUsage(pid);
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


// Case where it sends one message
void sendSingleMessage(std::vector<HWND> windows, const std::string& emotes, const std::string& msg)
{
    int wHoldPerClient = 18;
    int delayBetweenClients = 18;


    std::string greetings[] = {
        "yo wassup", "hola amigos", "salut les gars", "holla homies", "ciao belli",
        "hej hej", "aloha dude", "konnichiwa senpai", "wassup bro", "yo yo yo",
        "guten tag mates", "hej frens", "hola que pasa", "oi oi oi", "namaste fam"
    };

    std::uniform_int_distribution<> distDelay(0, 14);  // zero-based

    // Handle emotes that just need to be sent as one command
    if (emotes == "Dance")
    {
        setClipboardText("/e dance");
    }
    else if (emotes == "Dance2")
    {
        setClipboardText("/e dance2");
    }
    else if (emotes == "Dance3")
    {
        setClipboardText("/e dance3");
    }
    else if (emotes == "Point")
    {
        setClipboardText("/e point");
    }
    else if (emotes == "Shrug")
    {
        setClipboardText("/e shrug");
    }
    else if (emotes == "Cheer")
    {
        setClipboardText("/e cheer");
    }
    else if (emotes == "Stadium")
    {
        setClipboardText("/e Stadium");
    }
    else if (emotes == "Laugh")
    {
        setClipboardText("/e laugh");
    }
    else if (emotes == "Wave")
    {
        for (HWND hwnd : windows)
        {
            std::string greet = greetings[distDelay(gen)];
            setClipboardText(greet);
            Sleep(50); // ensure clipboard is ready

            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
            SetActiveWindow(hwnd);
            Sleep(wHoldPerClient);  // give time for focus

            BlockInput(TRUE);
            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);
            BlockInput(FALSE);

            Sleep(delayBetweenClients);
        }

        // Optional second message (e.g., the emote after the greeting)
        setClipboardText("/e hello");
        Sleep(50);
        for (HWND hwnd : windows)
        {
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
            SetActiveWindow(hwnd);
            Sleep(wHoldPerClient);

            BlockInput(TRUE);
            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
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
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        SetActiveWindow(hwnd);
        Sleep(wHoldPerClient);

        BlockInput(TRUE);
        pressSlash(wHoldPerClient);
        Sleep(3);
        pressCtrlV(wHoldPerClient);
        Sleep(3);
        pressEnter(wHoldPerClient);
        Sleep(3);
        BlockInput(FALSE);

        Sleep(delayBetweenClients);
    }
}


// Spam Messages
void spamMessages(int durationMs, const std::string& emotes, const std::string& msg)
{
    if (emotes == "Dance")
    {
        setClipboardText("/e dance");
        sleepMilli(durationMs);
        pressSlash(durationMs);
        pressCtrlV(durationMs);
        pressEnter(durationMs);
        setClipboardText("/e dance3");
        sleepMilli(durationMs);
        pressSlash(durationMs);
        pressCtrlV(durationMs);
        pressEnter(durationMs);
    }
    else
    {
        setClipboardText(msg);
    }
    pressSlash(durationMs);
    pressCtrlV(durationMs);
    pressEnter(durationMs);
    sleepMilli(durationMs);
}

// Undance
void Undance(std::vector<HWND> windows, int durationMs)
{
    for (HWND hwnd : windows)
    {
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        SetActiveWindow(hwnd);
        pressSpace(durationMs);
        sleepMilli(durationMs);
    }
}

// Sidebar update
void sideBarUpdater(HWND hwnd)
{
    size_t currentAmountOfInstances = 0;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::vector<HWND> instances = getRobloxWindows();
        std::vector<HWND> instancesToAttach;
        if (instances.size() != currentAmountOfInstances)
        {
            currentAmountOfInstances = instances.size();
            size_t difference = instances.size() - currentAmountOfInstances;

            PostMessage(hwnd, WM_UPDATE_SIDEBAR, 0, 0);
            if (instances.size() < currentAmountOfInstances) return;
            size_t startIndex = instances.size() - difference;

            for (size_t i = startIndex; i < instances.size(); ++i)
            {
                instancesToAttach.push_back(instances[i]);
            }

            onStart(instancesToAttach.size(), instancesToAttach);
        }
    }
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
            if (detectProccesEnd(pidsCleaned)) return;
            DWORD pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
            SetActiveWindow(hwnd);
            sleepMilli(20);
            pressSpace(20);     // Simulate jump
            sleepMilli(100);
            manageThreadOperations(pid, true); // Freezes specfic client
            sleepMilli(40); // focus
        }
        BlockInput(FALSE);

        for (int x = 0; x < freezeDurationMs; x += 100)
        {
            if (detectProccesEnd(pidsCleaned)) return;
            Sleep(100);
        }

        for (DWORD pid : pidsCleaned)
            manageThreadOperations(pid, false);
    }
}

// Convert Buffer to string (credits to chatgpt u came in cluch icl saved my ahh)
std::string convertChatText()
{
    const int BUF_SIZE = 1024;
    wchar_t buffer[BUF_SIZE] = { 0 };
    std::string chatText;

    if (!hChatTextbox) {
        return "bro";
    }

    GetWindowText(hChatTextbox, buffer, BUF_SIZE);

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    if (size_needed > 0) {
        chatText.resize(size_needed);
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &chatText[0], size_needed, nullptr, nullptr);

        if (!chatText.empty() && chatText.back() == '\0') {
            chatText.pop_back();
        }
    }

    return chatText;
}

// Main function
void mainFunc(const std::string& Mode)
{
    AllowSetForegroundWindow(ASFW_ANY);
    std::vector<HWND> windows = getRobloxWindows();
    size_t count = windows.size();

    if (count == 0)
    {
        MessageBox(NULL, L"No Roblox clients found.", L"Error", MB_ICONERROR); BlockInput(FALSE);
        return;
    }

    while (true)
    {
        if (exitFlag.load()) { ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }

        // Actions which are only peformed once or twice (CREDITS TO CHATGPT FOR CONVERTING BUFFER TO STRING (ICL I HAD NO IDEA HOW TO DO IT LMAO)
        if (Mode == "Send Message") { sendSingleMessage(windows, "None", convertChatText()); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Dance") { sendSingleMessage(windows, "Dance", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Dance2") { sendSingleMessage(windows, "Dance2", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Dance3") { sendSingleMessage(windows, "Dance3", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Undance") { Undance(windows, 35); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Wave") { sendSingleMessage(windows, "Wave", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Point") { sendSingleMessage(windows, "Point", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Stadium") { sendSingleMessage(windows, "Stadium", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Cheer") { sendSingleMessage(windows, "Cheer", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Laugh") { sendSingleMessage(windows, "Laugh", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Shrug") { sendSingleMessage(windows, "Shrug", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }
        if (Mode == "Freeze") { freezeAllClients(true); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); BlockInput(FALSE); return; }

        // Actions that are peformed until you click e + shift
        for (HWND hwnd : windows)
        {
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
            SetActiveWindow(hwnd);
            std::uniform_int_distribution<> distHold(33, 38);
            std::uniform_int_distribution<> distDelay(2, 5);
            int wHoldPerClient = distHold(gen);
            int delayBetweenClients = distDelay(gen);

            if (Mode == "Walk")
            {
                pressW(wHoldPerClient);
            }
            else if (Mode == "Walk Backward")
            {
                pressS(wHoldPerClient);
            }
            else if (Mode == "Walk Right")
            {
                pressD(wHoldPerClient);
            }
            else if (Mode == "Walk Left")
            {
                pressA(wHoldPerClient);
            }
            else if (Mode == "Jump")
            {
                pressSpace(wHoldPerClient);
            }
            else if (Mode == "Slow Walk")
            {
                wHoldPerClient = 1;
                delayBetweenClients = 1;
                pressW(wHoldPerClient);
            }
            else if (Mode == "Spam")
            {
                spamMessages(wHoldPerClient, "None", convertChatText());
            }
            else if (Mode == "Dance Loop")
            {
                spamMessages(23, "Dance", "None");
            }
            sleepMilli(delayBetweenClients);
        }
    }
}

HFONT hFont = CreateFont(
    18,
    0,
    0,
    0,
    FW_NORMAL,
    FALSE,
    FALSE,
    FALSE,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_SWISS,
    L"Segoe UI");

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_WALK:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk")).detach();
            break;
        case ID_WALK_RIGHT:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Right")).detach();
            break;
        case ID_WALK_LEFT:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Left")).detach();
            break;
        case ID_WALK_BACKWARD:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk Backward")).detach();
            break;
        case ID_JUMP:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Jump")).detach();
            break;
        case ID_SLOW_WALK:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Slow Walk")).detach();
            break;
        case ID_SEND_MESSAGE:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Send Message")).detach();
            break;
        case ID_SPAM_MESSAGE:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Spam")).detach();
            break;
        case ID_DANCE:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Dance")).detach();
            break;
        case ID_DANCE2:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Dance2")).detach();
            break;
        case ID_DANCE3:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Dance3")).detach();
            break;
        case ID_DANCELOOP:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Dance Loop")).detach();
            break;
        case ID_UNDANCE:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Undance")).detach();
            break;
        case ID_POINT:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Point")).detach();
            break;
        case ID_WAVE:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Wave")).detach();
            break;
        case ID_FREEZE:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Freeze")).detach();
            break;
        case ID_SHRUG:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Shrug")).detach();
            break;
        case ID_LAUGH:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Laugh")).detach();
            break;
        case ID_STADIUM:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Stadium")).detach();
            break;
        case ID_CHEER:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Cheer")).detach();
            break;
        default:
            break;
        }
        return 0; 

    case WM_CLOSE:
        BlockInput(FALSE);
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;
        SetBkColor(hdc, RGB(40, 40, 40));           // dark grey background
        SetTextColor(hdc, RGB(255, 255, 255));      // white text
        static HBRUSH hBrush = CreateSolidBrush(RGB(40, 40, 40));
        return (INT_PTR)hBrush;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(40, 40, 40)));
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_UPDATE_SIDEBAR:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        const int EDIT_WIDTH = 150;
        const int EDIT_HEIGHT = 25;
        const int EDIT_MARGIN = 5;
        int margin = 3;

        int y = 10;
        int x = rc.right - EDIT_WIDTH - margin;

        if (listOfUI.size() != 0)
        {
            for (HWND instance : listOfUI)
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
            listOfUI.push_back(clientDisplay);
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
        L"Moon Client Controller V3 - by Noah",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 680, 430,
        NULL, NULL, hInstance, NULL
    );

    std::thread(exitDetection).detach(); // Run Exit Detector
    std::thread(sideBarUpdater, hwnd).detach(); // Run SideBar Manager UI

    g_MainWindow = hwnd;

    if (!hwnd) return 0;

    const int BUTTON_WIDTH = 120;
    const int BUTTON_HEIGHT = 30;
    const int BUTTON_SPACING_X = 15;
    const int BUTTON_SPACING_Y = 10;
    const int START_X = 20;
    const int START_Y = 20;

    const int TEXTBOX_HEIGHT = 80;
    const int TEXTBOX_WIDTH = BUTTON_WIDTH * 3 + BUTTON_SPACING_X * 2;

    int x = START_X;
    int y = START_Y;
    int buttonsInRow = 0;

    auto placeButton = [&](LPCWSTR label, int id) {
        HWND button = CreateWindow(L"BUTTON", label,
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            x, y, BUTTON_WIDTH, BUTTON_HEIGHT,
            hwnd, (HMENU)id, hInstance, NULL);

        SendMessage(button, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

        buttonsInRow++;
        x += BUTTON_WIDTH + BUTTON_SPACING_X;
        if (buttonsInRow == 3) {
            buttonsInRow = 0;
            x = START_X;
            y += BUTTON_HEIGHT + BUTTON_SPACING_Y;
        }

        if (label == L"Cheer")
        {
            int textboxX = START_X;
            int textboxY = y + BUTTON_HEIGHT + BUTTON_SPACING_Y;
            hChatTextbox = CreateWindow(
                L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                textboxX, textboxY, TEXTBOX_WIDTH, TEXTBOX_HEIGHT,
                hwnd, (HMENU)ID_CHATBOX, hInstance, NULL);

            int buttonX = textboxX + TEXTBOX_WIDTH + BUTTON_SPACING_X;
            int buttonY = textboxY;

            HWND broadcastButton = CreateWindow(L"BUTTON", L"Broadcast Msg",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT,
                hwnd, (HMENU)ID_SEND_MESSAGE, hInstance, NULL);

            HWND spamButton = CreateWindow(L"BUTTON", L"Spam Msg",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                buttonX, buttonY + BUTTON_HEIGHT + BUTTON_SPACING_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                hwnd, (HMENU)ID_SPAM_MESSAGE, hInstance, NULL);

            SendMessage(broadcastButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            SendMessage(spamButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            SendMessage(hChatTextbox, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        }
        };

    // Place Buttons
    std::vector<std::pair<LPCWSTR, int>> buttons = {
       {L"Walk", ID_WALK},
       {L"Walk (backward)", ID_WALK_BACKWARD},
       {L"Walk (right)", ID_WALK_RIGHT},
       {L"Walk (left)", ID_WALK_LEFT},
       {L"Slow Walk", ID_SLOW_WALK},
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
    };

    for (auto& btn : buttons)
    {
        placeButton(btn.first, btn.second);
    }

    ShowWindow(hwnd, nCmdShow);
    MessageBoxW(hwnd,
        L"Moon Client Controller V3\n\nCreated by: Noah\nGitHub: https://github.com/thanknoah/Moon-Instance-Controller\nSpecial Thanks: Unknown",
        L"Credits",
        MB_OK | MB_ICONINFORMATION);

    size_t start = 0;
    onStart(start, {});


    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
