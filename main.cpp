// GUI & Copy and paste script credits to unknown.e, Code Logic Written by Noah

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
#define ID_EXIT 6
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distHold(33, 38);   // W key hold between 30-40 ms
std::uniform_int_distribution<> distDelay(2, 5);

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
            sleepSec(2);
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

// Credits to Unknown for clipboard script and pressCtrlV
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
void sendSingleMessage(std::vector<HWND> windows)
{
    // Delay time
    int wHoldPerClient = 30;
    int delayBetweenClients = 50;

    // PREPERATION
    setClipboardText("Hello, we were jumping over the fence yestedasy");
 
    for (HWND hwnd : windows) // Since this function isnt suppossed to be while looped because it completes one task, then we use for loop and then return to end : )
    {
        // Actually spamming it (manually setting focus here because its not a while loop)
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        SetActiveWindow(hwnd);
        pressSlash(wHoldPerClient);
        pressCtrlV(wHoldPerClient);
        pressEnter(wHoldPerClient);
        sleepMilli(delayBetweenClients);
    }
}

// Spam Messages
void spamMessages(int durationMs)
{
    // PREPERATION
    setClipboardText("Hello, we were jumping over the fence yestedasy");

    // Actually spamming it
    pressSlash(durationMs);
    pressCtrlV(durationMs);
    pressEnter(durationMs);
    sleepMilli(durationMs);
}

// Main function
void mainFunc(const std::string& Mode) 
{
    AllowSetForegroundWindow(ASFW_ANY);
    std::vector<HWND> windows = getRobloxWindows();
   
    // filter out non-roblox instances with "roblox" in them
    for (auto it = windows.begin(); it != windows.end(); )
    {
        wchar_t title[256];
        const wchar_t* robloxName = L"Roblox";
        GetWindowText(*it, title, sizeof(title) / sizeof(wchar_t));

        if (wcscmp(title, robloxName) != 0)
        {
            it = windows.erase(it);
        }
        else
        {
            ++it;
        }
    }

    size_t count = windows.size();
    if (count == 0)
    {
        MessageBox(NULL, L"No Roblox clients found.", L"Error", MB_ICONERROR);
        return;
    }

    while (true) 
    {
        if (exitFlag.load()) return;
        if (Mode == "Send Message") { sendSingleMessage(windows); return; }
        for (HWND hwnd : windows) 
        {
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
            SetActiveWindow(hwnd);
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
                spamMessages(wHoldPerClient);
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
        case ID_EXIT:
            PostQuitMessage(0);
            break;
        }
        return 0;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC: 
    {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }
    case WM_PAINT: 
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
        EndPaint(hwnd, &ps);
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
    std::thread(exitDetection).detach();
    const wchar_t CLASS_NAME[] = L"SimpleWin32GUI";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Roblox Client Controller",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 500,  // <- Bigger window
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    const int BUTTON_WIDTH = 160;
    const int BUTTON_HEIGHT = 40;
    const int BUTTON_SPACING = 20;
    const int START_Y = 20;

    int y = START_Y;

    CreateWindow(L"BUTTON", L"Walk", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_WALK, hInstance, NULL);
    y += BUTTON_HEIGHT + BUTTON_SPACING;

    CreateWindow(L"BUTTON", L"Slow Walk", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_SLOW_WALK, hInstance, NULL);
    y += BUTTON_HEIGHT + BUTTON_SPACING;

    CreateWindow(L"BUTTON", L"Jump", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_JUMP, hInstance, NULL);
    y += BUTTON_HEIGHT + BUTTON_SPACING;

    CreateWindow(L"BUTTON", L"Broadcast Message", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_SEND_MESSAGE, hInstance, NULL);
    y += BUTTON_HEIGHT + BUTTON_SPACING;

    CreateWindow(L"BUTTON", L"Spam Message", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_SPAM_MESSAGE, hInstance, NULL);
    y += BUTTON_HEIGHT + BUTTON_SPACING;

    CreateWindow(L"BUTTON", L"Exit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_EXIT, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
