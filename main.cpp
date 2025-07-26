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


// Public Variables
std::atomic<bool> exitFlag(false);
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
#define ID_EXIT 13
#define WM_UPDATE_SIDEBAR (WM_USER + 1)
std::random_device rd;
std::mt19937 gen(rd());
HWND g_MainWindow = NULL;
HINSTANCE g_hInstance = NULL;

// Get total amount of Roblox Instances Open
std::vector<HWND> getRobloxWindows()
{
    std::vector<HWND> instances;
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            if (!IsWindowVisible(hwnd)) return TRUE;
            wchar_t title[256];
            GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));

            if (wcsstr(title, L"Roblox"))
            {
                ((std::vector<HWND>*)lParam)->push_back(hwnd);
            }


            return TRUE;
        }, (LPARAM)&instances);
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
        }
        else if (!combination)
        {
            wasPressed = false;
        }

        sleepMilli(10); // Run Every 10 milliseconds (detector)
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
void sendSingleMessage(std::vector<HWND> windows, const std::string& emotes, const std::string& message)
{
    int wHoldPerClient = 30;
    int delayBetweenClients = 50;
 
    for (HWND hwnd : windows)
    {
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
        else if (emotes == "Wave")
        {
            setClipboardText("/e wave");
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
            sleepMilli(wHoldPerClient);
            SetActiveWindow(hwnd);
            pressSlash(wHoldPerClient);
            pressCtrlV(wHoldPerClient);
            pressEnter(wHoldPerClient);

            std::string greetings[] = {
            "yo wassup",
            "hola amigos",
            "salut les gars",
            "holla homies",
            "ciao belli",
            "hej hej",
            "aloha dude",
            "konnichiwa senpai",
            "wassup bro",
            "yo yo yo",
            "guten tag mates",
            "hej frens",
            "hola que pasa",
            "oi oi oi",
            "namaste fam"
            };

            std::uniform_int_distribution<> distDelay(1, 14);
            int i = distDelay(gen);
            setClipboardText(greetings[i]);
        }
        else
        {
            setClipboardText(message);
        }
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        sleepMilli(wHoldPerClient);
        SetActiveWindow(hwnd);
        pressSlash(wHoldPerClient);
        pressCtrlV(wHoldPerClient);
        pressEnter(wHoldPerClient);
        sleepMilli(delayBetweenClients);
    }
}

// Spam Messages
void spamMessages(int durationMs, const std::string& emotes)
{
    if (emotes == "Dance")
    {
        setClipboardText("/e dance");
        sleepMilli(durationMs);
        pressSlash(durationMs);
        pressCtrlV(durationMs);
        pressEnter(durationMs);
        setClipboardText("/e dance2");
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
        setClipboardText("Hello, we were jumping over the fence yestedasy");

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
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        PostMessage(hwnd, WM_UPDATE_SIDEBAR, 0, 0);
    }
}

// Main function
void mainFunc(const std::string& Mode) 
{
    AllowSetForegroundWindow(ASFW_ANY);
    std::vector<HWND> windows = getRobloxWindows();
    for (auto it = windows.begin(); it != windows.end(); )
    {
        wchar_t title[256];
        const wchar_t* robloxName = L"Roblox";
        GetWindowText(*it, title, sizeof(title) / sizeof(wchar_t));

        if (wcscmp(title, robloxName) != 0) { it = windows.erase(it); }
        if (wcscmp(title, robloxName) == 0) { ++it; }
    }

    size_t count = windows.size();

    if (count == 0)
    {
        MessageBox(NULL, L"No Roblox clients found.", L"Error", MB_ICONERROR);
        return;
    }

    while (true) 
    {
        if (exitFlag.load()) { ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); return; }

        // Actions which are only peformed once or twice
        if (Mode == "Send Message") { sendSingleMessage(windows, "None", "Hello, we were jumping over the fence yestedasy"); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); return; }
        if (Mode == "Dance") { sendSingleMessage(windows, "Dance", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); return; }
        if (Mode == "Dance2") { sendSingleMessage(windows, "Dance2", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); return; }
        if (Mode == "Dance3") { sendSingleMessage(windows, "Dance3", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); return; }
        if (Mode == "Undance") { Undance(windows, 35); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); return; }
        if (Mode == "Wave") { sendSingleMessage(windows, "Wave", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); return; }
        if (Mode == "Point") { sendSingleMessage(windows, "Point", ""); ShowWindow(g_MainWindow, SW_RESTORE); SetForegroundWindow(g_MainWindow); return; }

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
                spamMessages(wHoldPerClient, "None");
            }
            else if (Mode == "Dance Loop")
            {
                spamMessages(20, "Dance");
            }


            sleepMilli(delayBetweenClients);
        }
    }
}

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_WALK:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc, std::string("Walk")).detach();         
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
        case ID_EXIT:
            PostQuitMessage(0);
            break;
        }
        return 0;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkColor(hdcStatic, RGB(40, 40, 40));          // dark gray background
        SetTextColor(hdcStatic, RGB(255, 255, 255));     // white text
        static HBRUSH hBrush = CreateSolidBrush(RGB(40, 40, 40));
        return (INT_PTR)hBrush;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_UPDATE_SIDEBAR:
    {
        std::vector<HWND> listOfClientsUI;

        // Destroy old controls
        for (HWND h : listOfClientsUI)
            DestroyWindow(h);
        listOfClientsUI.clear();

        // Fetch updated clients - adapt to your actual function
        std::vector<HWND>clients = getRobloxWindows();

        const int SIDEBAR_WIDTH = 180;
        const int SIDEBAR_PADDING = 10;
        const int SIDEBAR_X = 700 - SIDEBAR_WIDTH - SIDEBAR_PADDING;
        const int SIDEBAR_START_Y = 20;
        const int SIDEBAR_LINE_HEIGHT = 30;

        for (auto it = clients.begin(); it != clients.end(); )
        {
            wchar_t title[256];
            const wchar_t* robloxName = L"Roblox";
            GetWindowText(*it, title, sizeof(title) / sizeof(wchar_t));

            if (wcscmp(title, robloxName) != 0) { it = clients.erase(it); }
            if (wcscmp(title, robloxName) == 0) { ++it; }
        }

        for (int i = 0; i < (int)clients.size(); i++) {
            wchar_t labelText[50];
            wsprintf(labelText, L"Connected Client %d", clients[i]);

            if (i <= 1)
            {
                if (clients[i] == clients[i - 1]) continue;
            }

            HWND label = CreateWindowEx(
                0, L"STATIC", labelText,
                WS_VISIBLE | WS_CHILD,
                SIDEBAR_X, SIDEBAR_START_Y + (SIDEBAR_LINE_HEIGHT * i),
                SIDEBAR_WIDTH, 25,
                hwnd, NULL, g_hInstance, NULL);

            if (label != NULL)
                listOfClientsUI.push_back(label);
        }

        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"SimpleWin32GUI";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    g_hInstance = hInstance;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST,  // ✅ Always on top
        CLASS_NAME,
        L"Roblox Client Controller",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 500,
        NULL, NULL, hInstance, NULL
    );

    std::thread(exitDetection).detach(); // Run Exit Detector
    std::thread(sideBarUpdater, hwnd).detach(); // Run SideBar Manager UI

    g_MainWindow = hwnd;

    if (!hwnd) return 0;

    const int BUTTON_WIDTH = 130;         // reduced from 160
    const int BUTTON_HEIGHT = 35;         // reduced from 40
    const int BUTTON_SPACING_X = 15;      // tighter horizontal spacing
    const int BUTTON_SPACING_Y = 15;      // tighter vertical spacing

    int startX = 20;
    int startY = 20;
    int x = startX;
    int y = startY;

    int buttonsInRow = 0;
    auto placeButton = [&](LPCWSTR label, int id) {
        CreateWindow(L"BUTTON", label, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            x, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)id, hInstance, NULL);

        x += BUTTON_WIDTH + BUTTON_SPACING_X;
        buttonsInRow++;

        if (buttonsInRow == 3) {
            buttonsInRow = 0;
            x = startX;
            y += BUTTON_HEIGHT + BUTTON_SPACING_Y;
        }
        };

    // Place Buttons

    placeButton(L"Walk", ID_WALK);
    placeButton(L"Slow Walk", ID_SLOW_WALK);
    placeButton(L"Jump", ID_JUMP);
    placeButton(L"Broadcast Msg", ID_SEND_MESSAGE); // shortened label
    placeButton(L"Spam Msg", ID_SPAM_MESSAGE);      // shortened label
    placeButton(L"Dance", ID_DANCE);
    placeButton(L"Dance 2", ID_DANCE2);
    placeButton(L"Dance 3", ID_DANCE3);
    placeButton(L"Zombie Dance", ID_DANCELOOP);
    placeButton(L"Undance", ID_UNDANCE);
    placeButton(L"Wave", ID_WAVE);
    placeButton(L"Point", ID_POINT);
    placeButton(L"Exit", ID_EXIT);


    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
