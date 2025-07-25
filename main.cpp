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
#include <windows.h>

// Public Variables
std::atomic<bool> exitFlag(false);
#define ID_WALK 1
#define ID_JUMP 2
#define ID_EXIT 3
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distHold(30, 40);   // W key hold between 30-40 ms
std::uniform_int_distribution<> distDelay(1, 5);

// Sleep simplified function
void sleepMilli(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

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
void pressW(int durationMs) {
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

// Press Space
void pressSpace(int durationMs) {
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

// Main function
void mainFunc() {
    AllowSetForegroundWindow(ASFW_ANY);

    std::vector<HWND> windows = getRobloxWindows();
    std::thread watcher(exitDetection);

    size_t count = windows.size();
    if (count == 0) {
        // No Roblox clients found
    }

    while (true) {
        if (exitFlag.load()) break;
        for (HWND hwnd : windows) {
            int wHoldPerClient = distHold(gen);
            int delayBetweenClients = distDelay(gen);

            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
            SetActiveWindow(hwnd);
            sleepMilli(10);
            pressW(wHoldPerClient);
            sleepMilli(delayBetweenClients);
        }
    }

    watcher.join();
}

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_WALK:
            ShowWindow(hwnd, SW_MINIMIZE);
            std::thread(mainFunc).detach();
            break;
        case ID_JUMP:
            break;
        case ID_EXIT:
            PostQuitMessage(0);
            break;
        }
        return 0;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }
    case WM_PAINT: {
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

// Main GUI
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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

    CreateWindow(L"BUTTON", L"Walk", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 20, 150, 40, hwnd, (HMENU)ID_WALK, hInstance, NULL);

    CreateWindow(L"BUTTON", L"Jump", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 80, 150, 40, hwnd, (HMENU)ID_JUMP, hInstance, NULL);

    CreateWindow(L"BUTTON", L"Exit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 300, 150, 40, hwnd, (HMENU)ID_EXIT, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
